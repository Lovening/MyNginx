
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
#include <sys/socket.h>
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"

//构造函数
CSocket::CSocket()
{
    m_ListenPortCount = 1;  
}

//释放函数
CSocket::~CSocket()
{
    //释放必须的内存
    std::vector<lpngx_listening_t>::iterator pos;	
	for(pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); ++pos) //vector
	{		
		delete (*pos); 
	}//end for
	m_ListenSocketList.clear(); 
}

//初始化函数【fork()子进程之前干这个事】
bool CSocket::Initialize()
{
    return ngx_open_listening_sockets();
}

//监听端口【支持多个端口】
//在创建worker进程之前就要执行这个函数；
bool CSocket::ngx_open_listening_sockets()
{
   CConfig *pconfig = CConfig::GetInstance();
   m_ListenPortCount = pconfig->GetIntDefault("ListenPortCount", m_ListenPortCount);

   int isock;
   struct sockaddr_in serv_addr;
   int iport;
   char strinfo[100];

   //初始化 相关
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

   for (int i = 0; i < m_ListenPortCount; ++i)
   {
       isock = socket(AF_INET, SOCK_STREAM, 0);
       if (-1 == isock)
       {
           ngx_log_stderr(errno, "CSocket::Instance()中socket()失败， i=%d", i);
           return false;
       }

       //setsockopt（）:设置一些套接字参数选项；主要是解决TIME_WAIT这个状态导致bind()失败的问题
       int reuseaddr = 1;
       if (-1 == setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuseaddr, sizeof(int)))
       {
             ngx_log_stderr(errno,"CSocekt::Initialize()中setsockopt(SO_REUSEADDR)失败,i=%d.",i);
            close(isock);                                           
            return false;
       }

       if (!setnonblocking(isock))
       {
           ngx_log_stderr(errno,"CSocekt::Initialize()中setnonblocking()失败,i=%d.",i);
            close(isock);
            return false;
       }
       
       memset(strinfo, 0, 100);
       sprintf(strinfo, "ListenPort%d", i);
       iport = pconfig->GetIntDefault(strinfo, 10000);
       serv_addr.sin_port = htons((in_port_t)iport);

       if (-1 == bind(isock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
       {
           ngx_log_stderr(errno, "CSocket::Initalize()中bind()失败！,i=%d", i);
           close(isock);
           return false;
       }

       if (-1 == listen(isock, NGX_LISTEN_BACKLOG))
       {
            ngx_log_stderr(errno,"CSocekt::Initialize()中listen()失败,i=%d.",i);
            close(isock);
            return false;
       }

       lpngx_listening_t plisten_socket_item = new ngx_listening_t;
       memset(plisten_socket_item, 0, sizeof(ngx_listening_t));
       plisten_socket_item->port = iport;
       plisten_socket_item->fd = isock;
       ngx_log_error_core(NGX_LOG_INFO, 0, "监听%d端口成功!", iport);
       m_ListenSocketList.push_back(plisten_socket_item);

   }


   return true;
}

bool CSocket::setnonblocking(int sockfd) 
{    
    //方式一：
    int inb = 1;    //0:清除，1：设置
    if (-1 == ioctl(sockfd, FIONBIO, &inb))
    {
        return false;
    }

    return true;

    // //方式二：
    // int opts = fcntl(sockfd, F_GETFL);
    // if ( opts < -1)
    // {
    //     return false;
    // }
    // op |= O_NONBLOCK;
    // if (fcntl(sockfd, F_SETFL, opts) < 0)
    // {
    //     return false;
    // }

    // return true;
}

//关闭socket
void CSocket::ngx_close_listening_sockets()
{
   for (int i = 0; i < m_ListenPortCount; ++i)
   {
       close(m_ListenSocketList[i]->fd);
       ngx_log_error_core(NGX_LOG_INFO, 0, "关闭监听端口%d", m_ListenSocketList[i]->port);
   }
}
