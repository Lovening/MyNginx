
#ifndef __NGX_GBLDEF_H__
#define __NGX_GBLDEF_H__

//一些比较通用的定义放在这里
#include <signal.h>
#include "ngx_c_slogic.h"
#include "ngx_c_threadpool.h"

//结构定义
typedef struct
{
	char ItemName[50];
	char ItemContent[500];
}CConfItem,*LPCConfItem;

//和运行日志相关 
typedef struct
{
	int    log_level;   //日志级别 或者日志类型，ngx_macro.h里分0-8共9个级别
	int    fd;          //日志文件描述符

}ngx_log_t;


//外部全局量声明
extern int   g_iargc; //参数个数
extern char  **g_ppargv;
extern char  *g_penvironmem; 
extern int   g_ienvironlen; 
extern int   g_iargvmemlen;

extern ngx_log_t  g_ngx_log;

extern pid_t	g_ngx_ppid; 			
extern pid_t    g_ngx_pid;
extern int		g_isdaemon;
extern int		g_iprocesstype;

extern sig_atomic_t  g_ngx_reap;

extern  CLogicSocket g_socket; 

extern CThreadPool g_threadpool; 

#endif
