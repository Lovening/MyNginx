
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

//当新连接进入时调用
void CSocket::ngx_event_accept(lpngx_connection_t pold_conn)
{
    struct sockaddr      mysockaddr;
    socklen_t            socklen;
    int                  err;
    int                  level;
    int                  connfd;
    static int           use_accept4 = 1;
    lpngx_connection_t   pnew_conn;

    socklen = sizeof(mysockaddr);
    do
    {
        if (use_accept4)
        {
            connfd = accept4(pold_conn->fd, &mysockaddr, &socklen, SOCK_NONBLOCK);
        }
        else
        {
            connfd = accept(pold_conn->fd, &mysockaddr, &socklen);
        }

         if (-1 == connfd)  
         {
             err = errno;
             if (err == EAGAIN)
             {
                 return;
             }

             level = NGX_LOG_ALERT;
             if (err == ECONNABORTED) //ECONNRESET错误则发生在对方意外关闭套接字
             {
                 level = NGX_LOG_ERR;
             }
             else if (err == EMFILE || err == ENFILE) //系统资源限制
             {
                level = NGX_LOG_CRIT;
             }
            ngx_log_error_core(level,errno,"CSocekt::ngx_event_accept()中accept4()失败!");

            if (use_accept4 && err ==ENOSYS)
            {
                use_accept4 = 0;
                continue;
            }
            return;
         }

        pnew_conn = ngx_get_connection(connfd);
        if (pnew_conn == NULL)
        {
            if(close(connfd) == -1)
            {
                ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_event_accept()中close(%d)失败!",connfd);                
            }
            return;
        }

        //拷贝客户端地址到连接对象
        memcpy(&pnew_conn->t_sockaddr, &mysockaddr, socklen);

        //测试将收到的地址弄成字符串，格式形如"192.168.1.126:40904"或者"192.168.1.126"
        u_char ipaddr[100]; 
        memset(ipaddr,0,sizeof(ipaddr));
        ngx_sock_ntop(&pnew_conn->t_sockaddr,1,ipaddr,sizeof(ipaddr)-10); //宽度给小点
        ngx_log_stderr(0,"ip信息为%s\n",ipaddr);

        if (!use_accept4)
        {
            if (!setnonblocking(connfd))
            {
                ngx_close_accepted_connection(pnew_conn);
                return;
            }
        }

        pnew_conn->plistening = pold_conn->plistening;
        pnew_conn->iwrite_ready = 1;
        pnew_conn->fp_rhandler = &CSocket::ngx_wait_request_handler;

        if (ngx_epoll_add_event(connfd,
                                1, 0,
                                EPOLLET,
                                EPOLL_CTL_ADD, 
                                pnew_conn) == -1)
            {
                ngx_close_accepted_connection(pnew_conn);
                return;
            }

    } while (false);
}

void CSocket::ngx_close_accepted_connection(lpngx_connection_t c)
{
   int fd = c->fd;
   ngx_free_connection(c);
   c->fd = -1;
   if (close(fd) == -1)
   {
        ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_close_accepted_connection()中close(%d)失败!",fd);  
   }
   
}
