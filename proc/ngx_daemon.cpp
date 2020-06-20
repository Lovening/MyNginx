//和守护进程相关

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>     //errno
#include <sys/stat.h>
#include <fcntl.h>


#include "ngx_func.h"
#include "ngx_macro.h"
#include "ngx_c_conf.h"

//守护进程初始化
//执行失败：返回-1，   子进程：返回0，父进程：返回1
int ngx_daemon()
{
    umask(0); 

   switch(fork())
   {
    case -1:
        ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon()中fork()失败！");
        return -1;
        break;
    case 0:
        //子进程
        break;
    default:
        return 1;
        break;
   }

    g_ngx_ppid = g_ngx_pid;
    g_ngx_pid = getpid();

    if (-1 == setsid())
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon()中setid()失败！");
        return -1;
    }

    //重定向
    int fd = open("/dev/null", O_RDWR);
    if (-1 == fd)
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon()中open()失败！");
        return -1;
    }

    if (-1 == dup2(fd, STDIN_FILENO))
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon()中dup2(STDIN_FILENO)失败！");
        return -1;
    }

    if(-1 == dup2(fd, STDOUT_FILENO))
    {
        ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon()中dup2(STDOUT_FILENO)失败！");
        return -1;
    }

    if (fd > STDERR_FILENO)
    {
        if (-1 == close(fd))
        {
            ngx_log_error_core(NGX_LOG_EMERG, errno, "ngx_daemon()close()失败！");
            return -1;
        }
    }

    return 0; 
}

