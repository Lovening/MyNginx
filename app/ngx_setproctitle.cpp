
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //env
#include <string.h>

#include "ngx_global.h"

void ngx_init_setproctitle()
{    
    //初始化环境变量
    g_penvironmem = new char[g_ienvironlen];
    memset(g_penvironmem, 0, g_ienvironlen);

    //拷贝内存
     char *ptmp = g_penvironmem;
     for (int i = 0; environ[i]; ++i)
     {
         size_t size = strlen(environ[i]) + 1;
         strcpy(ptmp, environ[i]);
         environ[i] = ptmp;
        ptmp += size;
     }
}

void ngx_setproctitle(const char *title)
{
    size_t titlesize = strlen(title);

    size_t argvslen = 0;
    if (titlesize >= g_iargvmemlen + g_ienvironlen)
        return;
    
    g_ppargv[1] = NULL;

    char *ptmp = g_ppargv[0];
    strcpy(ptmp, title);
    ptmp += titlesize;

    //清空之后的内存
    memset(ptmp, 0, argvslen + g_ienvironlen - titlesize);
}