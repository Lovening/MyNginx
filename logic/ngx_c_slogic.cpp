#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>    //uintptr_t
#include <stdarg.h>    //va_start....
#include <unistd.h>    //STDERR_FILENO等
#include <sys/time.h>  //gettimeofday
#include <time.h>      //localtime_r
#include <fcntl.h>     //open
#include <errno.h>     //errno
//#include <sys/socket.h>
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>
#include <pthread.h>   //多线程

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_memory.h"
#include "ngx_c_crc32.h"
#include "ngx_c_slogic.h"  
#include "ngx_logiccomm.h"  

//定义成员函数指针
typedef bool (CLogicSocket::*handler)(lpngx_connection_t pConn,             //连接池指针
                                      LPSTRUCT_MSG_HEADER pMsgHeader,        //消息头指针
                                      char *pPkgBody,                       //包体
                                      unsigned short iPkgBodyLen);          //包体长度

//用来保存 成员函数指针 的这么个数组
static const handler funHandlerList[] = 
{
    //数组前5个元素，保留，以备将来增加一些基本服务器功能
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    //开始处理具体的业务逻辑
    &CLogicSocket::HandleRegister,                         //【5】：实现具体的注册功能
    &CLogicSocket::HandleLogIn,                            //【6】：实现具体的登录功能
    //......其他待扩展，比如实现攻击功能，实现加血功能等等；

};
#define AUTH_TOTAL_COMMANDS sizeof(funHandlerList)/sizeof(handler) //整个命令有多少个，编译时即可知道

CLogicSocket::CLogicSocket()
{

}
CLogicSocket::~CLogicSocket()
{

}

bool CLogicSocket::Initialize()
{
    //做一些和本类相关的初始化工作
        
    return CSocket::Initialize();;
}

//处理收到的数据包
//pMsgBuf：消息头 + 包头 + 包体
void CLogicSocket::threadRecvProcFunc(char *pMsgBuf)
{          
    LPSTRUCT_MSG_HEADER pMsgHeader = (LPSTRUCT_MSG_HEADER)pMsgBuf;
    LPCOMM_PKG_HEADER pPkgHeader = (LPCOMM_PKG_HEADER)(pMsgBuf + m_imsghead_len);
    char *pPkgBody = NULL;
    unsigned short sPkgLen = ntohs(pPkgHeader->sPckLen);

    if (m_ipkghead_len == sPkgLen)
    {
        if (pPkgHeader->iCrc32 != 0)
        {
            return;
        }
        pPkgBody = NULL;
    }
    else
    {
        pPkgHeader->iCrc32 = ntohl(pPkgHeader->iCrc32);
        pPkgBody = (char *)(pMsgBuf + m_imsghead_len + m_ipkghead_len);
        
        //crc32校验
        int iCalCrc = CCRC32::GetInstance()->Get_CRC((unsigned char*)pPkgBody, sPkgLen - m_ipkghead_len);
        if (pPkgHeader->iCrc32 != iCalCrc)
        {
             ngx_log_stderr(0,"CLogicSocket::threadRecvProcFunc()中CRC错误，丢弃数据!");
            return;
        }
    }
    
    lpngx_connection_t pConn = pMsgHeader->pconn;       
    if(pConn->icur_sequence != pMsgHeader->icur_sequence)   
    {
        return; //客户端断开
    }
    
    unsigned short sMsgCode = ntohs(pPkgHeader->sMsgCode);
    if (sMsgCode >= AUTH_TOTAL_COMMANDS)
    {
        ngx_log_stderr(0,"CLogicSocket::threadRecvProcFunc()中imsgCode=%d消息码不对!",sMsgCode);
        return;
    }

    if (funHandlerList[sMsgCode] == NULL)
    {
        ngx_log_stderr(0,"CLogicSocket::threadRecvProcFunc()中imsgCode=%d消息码找不到对应的处理函数!",sMsgCode);
        return;
    }

    (this->*funHandlerList[sMsgCode])(pConn, pMsgHeader, pPkgBody, sPkgLen);

}

//----------------------------------------------------------------------------------------------------------
//处理各种业务逻辑
bool CLogicSocket::HandleRegister(lpngx_connection_t pConn,LPSTRUCT_MSG_HEADER pMsgHeader,char *pPkgBody,unsigned short iBodyLength)
{
    if (pPkgBody == NULL)
    {
        return false;
    }

    int iRecvLen = sizeof(STRUCT_REGISTER);
    if (iRecvLen != iBodyLength)
    {
        return false;
    }

    CLock lock(&pConn->logicPorcMutex);

    LPSTRUCT_REGISTER pRecvInfo = (LPSTRUCT_REGISTER)pPkgBody;
    //业务逻辑

    //给客户端返回数据时，一般也是返回一个结构，这个结构内容具体由客户端/服务器协商，这里我们就以给客户端也返回同样的 STRUCT_REGISTER 结构来举例
    CMemory  *pMemory = CMemory::GetInstance();
	CCRC32   *pCrc32 = CCRC32::GetInstance();

    int iSendLen = sizeof(STRUCT_REGISTER);
    char *pSendBuf = pMemory->AllocMemory(m_imsghead_len + m_ipkghead_len + iSendLen);

    //消息头
    memcpy(pSendBuf, pMsgHeader, m_imsghead_len);
    //包头
    LPCOMM_PKG_HEADER pPkgHeader = pSendBuf + m_imsghead_len;
    pPkgHeader->sPckLen = htons(m_ipkghead_len + iSendLen);
    pPkgHeader->sMsgCode = htons(_CMD_REGISTER);
    //包体
    LPSTRUCT_REGISTER pSendInfo = (LPSTRUCT_REGISTER)(pSendBuf + m_imsghead_len + m_ipkghead_len);
    pPkgHeader->iCrc32 = htonl(pCrc32->Get_CRC((unsigned char *)pSendInfo, iSendLen));

    //发送数据包




    ngx_log_stderr(0,"执行了CLogicSocket::HandleRegister()!");
    return true;
}
bool CLogicSocket::HandleLogIn(lpngx_connection_t pConn,LPSTRUCT_MSG_HEADER pMsgHeader,char *pPkgBody,unsigned short iBodyLength)
{
    ngx_log_stderr(0,"执行了CLogicSocket::HandleLogIn()!");
    return true;
}
