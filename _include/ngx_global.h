
#ifndef __NGX_GBLDEF_H__
#define __NGX_GBLDEF_H__

//一些比较通用的定义放在这里


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
extern char  **g_ppargv;
extern char  *g_penvironmem; 
extern int   g_ienvironlen; 

extern pid_t       g_ngx_pid;
extern ngx_log_t   g_ngx_log;

#endif
