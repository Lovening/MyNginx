﻿
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

//从连接池中获取一个空闲连接
lpngx_connection_t CSocket::ngx_get_connection(int isock)
{
    lpngx_connection_t pfront = m_pfree_connections;

    if (pfront == NULL)
    {
        ngx_log_stderr(0, "CSocket::ngx_get_connectionn()中空闲链表为空！");
        return NULL;
    }

    m_pfree_connections = pfront->pnext;
    --m_ifree_connection;

    //清空内存
    unsigned instance = pfront->instance;
    uint64_t icursequence = pfront->icur_sequence;

    memset(pfront, 0, sizeof(ngx_connection_t));
    pfront->fd = isock;
    pfront->icurstat = _PKG_HD_INIT;
    pfront->precvbuf = pfront->headinfo_arr;
    pfront->irest_recvlen = sizeof(COMM_PKG_HEADER);
    pfront->bnew_recvmem = false;
    pfront->pnewmem = NULL;

    pfront->instance = !instance;   //防止close()后，被复用
    pfront->icur_sequence = icursequence;
    ++pfront->icur_sequence;

    return pfront;
}

//归还参数c所代表的连接到到连接池中
void CSocket::ngx_free_connection(lpngx_connection_t c) 
{
     if(c->bnew_recvmem)
    {
        //我们曾经给这个连接分配过内存，则要释放内存        
        CMemory::GetInstance()->FreeMemory(c->pnewmem);
        c->pnewmem = NULL;
        c->bnew_recvmem = false;  //这行有用？
    }

    c->pnext = m_pfree_connections;                      

   //回收后，该值就增加1,以用于判断某些网络事件是否过期【一被释放就立即+1也是有必要的】
    ++c->icur_sequence;   

    m_pfree_connections = c;                           
    ++m_ifree_connection;        

}

void CSocket::ngx_close_connection(lpngx_connection_t c)
{
  
   if (close(c->fd) == -1)
   {
        ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_close_connection()中close(%d)失败!",c->fd);  
   }
   c->fd = -1;
   ngx_free_connection(c);
   
}

