
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

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"
#include "ngx_c_memory.h"

//来数据时候的处理，当连接上有数据来的时候，本函数会被ngx_epoll_process_events()所调用  ,官方的类似函数为ngx_http_wait_request_handler();
void CSocket::ngx_wait_request_handler(lpngx_connection_t c)
{    
    ssize_t irecv = RecvProc(c, c->precvbuf, c->irest_recvlen);
    if (irecv <= 0)
    return;

    if (c->icurstat == _PKG_HD_INIT)
    {
        if (irecv == m_ipkghead_len)
        {
            //收包头
            ngx_wait_request_hander_proc_p1(c);
            
        }
        else 
        {
            c->icurstat = _PKG_HD_RECVING;
            c->precvbuf += irecv;
            c->irest_recvlen -= irecv;
        }
    }
    else if (c->icurstat == _PKG_HD_RECVING)
    {
        if (c->irest_recvlen == irecv)
        {
            //收包头
            ngx_wait_request_hander_proc_p1(c);
        }
        else
        {
            c->precvbuf += irecv;
            c->irest_recvlen -= irecv;
        }
    }
    else if (c->icurstat == _PKG_BD_INIT)
    {
        if (c->irest_recvlen == irecv)
        {
            ngx_wait_request_handler_proc_plast(c);
        }
        else
        {
            c->icurstat = _PKG_BD_RECVING;
            c->precvbuf +=  irecv;
            c->irest_recvlen -= irecv;
        }
        
    }
    else if(c->icurstat == _PKG_BD_RECVING) 
    {
        if(c->irest_recvlen == irecv)
        {
            ngx_wait_request_handler_proc_plast(c);
        }
        else
        {
            c->precvbuf +=  irecv;
            c->irest_recvlen -= irecv;
        }
    }
    
}

void CSocket::ngx_wait_request_handler_proc_plast(lpngx_connection_t c)
{

    //保存至消息队列
    g_threadpool.InMsgRecvQueueAndSignal(c->pnewmem);

    c->bnew_recvmem = false;
    c->pnewmem = NULL;
    c->icurstat = _PKG_HD_INIT;
    c->precvbuf = c->headinfo_arr;
    c->irest_recvlen = m_ipkghead_len;      //重置接收长度

}

void CSocket::ngx_wait_request_hander_proc_p1(lpngx_connection_t c)
{
    LPCOMM_PKG_HEADER pPkgHeader = (LPCOMM_PKG_HEADER)c->headinfo_arr;
    unsigned short sPackageLen = ntohs(pPkgHeader->sPckLen);

    //校验
    if (sPackageLen < m_ipkghead_len
        || sPackageLen > _PKG_MAX_LENGTH - 1000)
    {
        //包错误
        c->icurstat = _PKG_HD_INIT;
        c->precvbuf = c->headinfo_arr;
        c->irest_recvlen = m_ipkghead_len;
    }
    else
    {
       
         char *pTmpBuffer  = (char *)CMemory::GetInstance()
                 ->AllocMemory(m_imsghead_len + sPackageLen,false);
        
        c->bnew_recvmem = true;
        c->pnewmem = pTmpBuffer;

        //填充
        LPSTRUCT_MSG_HEADER pTempMsgHeader = LPSTRUCT_MSG_HEADER(pTmpBuffer);
        pTempMsgHeader->pconn = c;
        pTempMsgHeader->icur_sequence = c->icur_sequence;

        //包头
        pTmpBuffer += m_imsghead_len;
        memcpy(pTmpBuffer, pPkgHeader, m_ipkghead_len);

        if (sPackageLen == m_ipkghead_len)
        {
            //无包体
            //插入消息队列
            ngx_wait_request_handler_proc_plast(c);
        }
        else
        {
            c->icurstat = _PKG_BD_INIT;
            c->precvbuf = pTmpBuffer + m_ipkghead_len;
            c->irest_recvlen = sPackageLen - m_ipkghead_len;
        }
    }
}

ssize_t CSocket::RecvProc(lpngx_connection_t c, char *buff, ssize_t buflen)
{
    ssize_t n;
    n = recv(c->fd, buff, buflen, 0);

    if(n == 0) 
    {
        ngx_close_connection(c);
        return -1;
    }
    else if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EAGAIN || errno == EWOULDBLOCK成立，发生错误！");
            return -1;
        }

        if (errno == EINTR)
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EINTR成立，发生错误！");
            return -1;
        }

        if (errno == ECONNRESET)
        {
            //非正常关闭错误
        }
        else
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中发生错误，我打印出来看看是啥错误！");
        }
        
        ngx_log_stderr(0,"连接被客户端 非 正常关闭！");
        ngx_close_connection(c);
        return -1;
    }

    return n;
}