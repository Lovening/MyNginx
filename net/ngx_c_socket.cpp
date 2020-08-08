
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

CSocket::CSocket()
{
    m_ilisten_port_count = 1;
    m_iworker_connections = 1;

    m_iepoll_handle = -1;
    m_pconnections = NULL;
    m_pfree_connections = NULL;

    m_iconnection = 0;
    m_ifree_connection = 0;

    m_ipkgheader_len = sizeof(COMM_PKG_HEADER);
    m_imsghead_len = sizeof(STUCT_MSG_HEADER);

}

CSocket::~CSocket()
{
    std::vector<lpngx_listening_t>::iterator pos;	
	for(pos = m_listen_socket_list.begin(); pos != m_listen_socket_list.end(); ++pos) //vector
	{		
		delete (*pos); 
	}
	m_listen_socket_list.clear(); 
}

//初始化函数【fork()子进程之前干这个事】
bool CSocket::Initialize()
{
    ReadConf();

    return ngx_open_listening_sockets();
}

void CSocket::ReadConf()
{
   CConfig *pconfig = CConfig::GetInstance();
   m_ilisten_port_count = pconfig->GetIntDefault("ListenPortCount", m_ilisten_port_count);
   m_iworker_connections = pconfig->GetIntDefault("worker_connections", m_iworker_connections);
}

//监听端口【支持多个端口】
//在创建worker进程之前就要执行这个函数；
bool CSocket::ngx_open_listening_sockets()
{
   int isock;
   struct sockaddr_in serv_addr;
   int iport;
   char strinfo[100];

   //初始化 相关
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

   for (int i = 0; i < m_ilisten_port_count; ++i)
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
             ngx_log_stderr(errno,"CSocket::Initialize()中setsockopt(SO_REUSEADDR)失败,i=%d.",i);
            close(isock);                                           
            return false;
       }

       if (!setnonblocking(isock))
       {
           ngx_log_stderr(errno,"CSocket::Initialize()中setnonblocking()失败,i=%d.",i);
            close(isock);
            return false;
       }
       
       memset(strinfo, 0, 100);
       sprintf(strinfo, "ListenPort%d", i);
       iport = CConfig::GetInstance()->GetIntDefault(strinfo, 10000);
       serv_addr.sin_port = htons((in_port_t)iport);

       if (-1 == bind(isock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
       {
           ngx_log_stderr(errno, "CSocket::Initalize()中bind()失败！,i=%d", i);
           close(isock);
           return false;
       }

       if (-1 == listen(isock, NGX_LISTEN_BACKLOG))
       {
            ngx_log_stderr(errno,"CSocket::Initialize()中listen()失败,i=%d.",i);
            close(isock);
            return false;
       }

       lpngx_listening_t plisten_socket_item = new ngx_listening_t;
       memset(plisten_socket_item, 0, sizeof(ngx_listening_t));
       plisten_socket_item->port = iport;
       plisten_socket_item->fd = isock;
       ngx_log_error_core(NGX_LOG_INFO, 0, "监听%d端口成功!", iport);
       m_listen_socket_list.push_back(plisten_socket_item);

   }
   
   if (m_listen_socket_list.size() <= 0)
       return false;

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
   for (int i = 0; i < m_ilisten_port_count; ++i)
   {
       close(m_listen_socket_list[i]->fd);
       ngx_log_error_core(NGX_LOG_INFO, 0, "关闭监听端口%d", m_listen_socket_list[i]->port);
   }
}


//--------------------------------------------------------------------
//(1)epoll功能初始化，子进程中进行 
int CSocket::ngx_epoll_init()
{
    //1）创建epoll
    m_iepoll_handle = epoll_create(m_iworker_connections);
    if (-1 == m_iepoll_handle)
    {
        ngx_log_stderr(errno, "CSocket::ngx_epoll_init()中epoll_create()失败.");
        exit(2);  //致命错误
    }

    //2)创建连接池
    m_iconnection = m_iworker_connections;
    m_pconnections = new ngx_connection_t[m_iconnection];

    int i = m_iconnection;
    lpngx_connection_t pnext = NULL;
    lpngx_connection_t pfront = m_pconnections;

    do
    {
        --i;

        pfront[i].pnext = pnext;
        pfront[i].fd = -1;
        pfront[i].instance = 1;
        pfront[i].icur_sequence = 0;

        pnext = &pfront[i];
        
    }while(i);    
    
    m_pfree_connections = pnext;
    m_ifree_connection = m_iconnection;

    //3）遍历所有监听socket【监听端口】，为每个监听socket增加一个 连接池中的连接
    std::vector<lpngx_listening_t>::iterator iter = m_listen_socket_list.begin();
    for (; iter != m_listen_socket_list.end(); ++iter)
    {
        lpngx_listening_t plistening = *iter;
        pfront = ngx_get_connection(plistening->fd);
        if (NULL == pfront)
        {
            ngx_log_stderr(errno, "CSocket::ngx_epoll_init()中ngx_get_connection()失败！");
            exit(2);
        }

        pfront->plistening = plistening;
        (*iter)->pconnection = pfront;

        pfront->fp_rhandler = &CSocket::ngx_event_accept;

        if (ngx_epoll_add_event(plistening->fd, 
                                1,0,
                                0, 
                                EPOLL_CTL_ADD, 
                                pfront) == -1)
            {
                exit(2);
            }

    }

    return 1;
}

//epoll增加事件
int CSocket::ngx_epoll_add_event(int fd,
                                int readevent,int writeevent,
                                uint32_t otherflag, 
                                uint32_t eventtype, 
                                lpngx_connection_t pconn
                                )
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    if (readevent == 1)
    {
        ev.events = EPOLLIN | EPOLLRDHUP;
    }
    else
    {
        
    }
    
    if (otherflag != 0)
    {
        ev.events |= otherflag;
    }

    ev.data.ptr = (void *)((uintptr_t)pconn | pconn->instance);

    if (epoll_ctl(m_iepoll_handle, eventtype, fd, &ev) == -1)
    {
        ngx_log_stderr(errno, "CSocket::ngx_epoll_add_event()中epoll_ctl(%d,%d,%d,%u,%u)失败！",
                fd, readevent, writeevent, otherflag, eventtype);

                return -1;
    }

    return 1;
}

//开始获取发生的事件消息,阻塞的时长，单位是毫秒；
int CSocket::ngx_epoll_process_events(int timer) 
{   
    int ievent = epoll_wait(m_iepoll_handle, m_event_arr, NGX_MAX_EVENTS, timer);

    if (-1 == ievent)   //有错误发生
    {
        if  (errno == EINTR)
        {
            ngx_log_error_core(NGX_LOG_INFO, errno, "CSocket::ngx_epoll_process_events()中epoll_wait()失败！");
            return 1;
        }
        else
        {
            ngx_log_error_core(NGX_LOG_ALERT, errno, "CSocket::ngx_epoll_process_events()epoll_wait()失败！");
            return 0;
        }
        
    }
    else if (0 == ievent)   //未收到事件
    {
        if (timer != -1) //超时
        {
            return 1;
        }

        ngx_log_error_core(NGX_LOG_ALERT, 0, "CSocket::ngx_epoll_process_events()中epoll_wait()未超时未返回任何事件！");
        return 0;
    }
    else
    {
        //正常收到事件
        lpngx_connection_t pconn = NULL;
        uintptr_t instance;//用于指针位操作
        uint32_t revents;
        for (int i = 0; i < ievent; ++i)
        {
            pconn = (lpngx_connection_t)(m_event_arr[i].data.ptr);
            instance = (uintptr_t)pconn & 1;
            pconn = (lpngx_connection_t)((uintptr_t)pconn & (uintptr_t)~1);

            //是否过期
            if (pconn->fd == -1)
            {
                ngx_log_error_core(NGX_LOG_DEBUG, 0, "CSocket::ngx_epoll_process_events()中遇到了fd=-1的过期事件:%p!", pconn);
                continue;
            }

            //是否过期
            if (pconn->instance != instance)
            {
                 ngx_log_error_core(NGX_LOG_DEBUG,0,"CSocekt::ngx_epoll_process_events()中遇到了instance值改变的过期事件:%p.",pconn); 
                 continue;
            }

            revents = m_event_arr[i].events;    //事件类型

            if (revents & (EPOLLERR | EPOLLHUP))
            {
                revents |= EPOLLIN | EPOLLOUT;
            }

            if (revents & EPOLLIN)
            {
                (this->*(pconn->fp_rhandler))(pconn);
            }

            if (revents & EPOLLOUT)
            {
                ngx_log_stderr(errno,"111111111111111111111111111111.");
            }
        }
    }

    return 1;
}
