#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ngx_func.h"
#include "ngx_c_conf.h"
#include "ngx_macro.h"

char    **g_ppargv = NULL;
char    *g_penvironmem = NULL; 
int     g_ienvironlen; 
int     g_iargvmemlen;
int     g_iargc;

pid_t   g_ngx_pid;           //当前进程的pid
pid_t	g_ngx_ppid; 
int 	g_isdaemon;		    //是否是守护进程
int     g_iprocesstype;

sig_atomic_t  g_ngx_reap;         //标记子进程状态变化[一般是子进程发来SIGCHLD信号表示退出],
                                //sig_atomic_t:系统定义的类型：访问或改变这些变量需要在计算机的一条指令内完成


static void freeresource();

int main(int argc, char* const argv[])
{
    //printf("Hello MyNginx!\n");
    int iexitcode = 0;

    //初始化变量
    g_ppargv = (char**)argv;
    g_iargc = argc;
    g_ngx_pid = getpid();
    for (int i = 0; i < argc; ++i)
    {
        g_iargvmemlen += strlen(argv[i]) + 1;
    }
     for (int i = 0; environ[i]; ++i)
    {
        g_ienvironlen += strlen(environ[i]) + 1;
    }

    g_ngx_log.fd = -1;                  //-1：表示日志文件尚未打开
    g_iprocesstype = NGX_PROCESS_MASTER;

    //读取配置文件
    CConfig* pConfig = CConfig::GetInstance();
    if (!pConfig->Load("nginx.conf"))
    {
        ngx_log_init();
        ngx_log_stderr(0, "配置文件[%s]载入失败，退出！", "nginx.conf");
        iexitcode = 2;
        goto lblexit;
        //exit(1);
    }

    //日志初始化
    ngx_log_init();

    //信号初始化
    if(ngx_init_signals() == -1)
    {
        iexitcode = 1;
        goto lblexit;
    }

    //移走环境变量
    ngx_init_setproctitle();

    //创建守护进程
    if (1 == pConfig->GetIntDefault("Daemon", 0))
    {
        int iresult = ngx_daemon();
        if (-1 == iresult)
        {
            iexitcode = 1;
            goto lblexit;
        }
        else if (1 == iresult)
        {
            //原始父进程，退出
            freeresource();
            iexitcode = 0;
            return iexitcode;
        }
        else
        {
            g_isdaemon = 1;
        }
    }

    ngx_master_process_cycle();

    // for (;;)
    // {
    //     printf("休息一秒！\n");
    //     sleep(1);
    // }

lblexit:
    ngx_log_stderr(0, "MyNginx Bye!");
    freeresource();
    return iexitcode;
}


static void freeresource()
{
    //释放环境变量
     if (g_penvironmem)
    {
        delete[] g_penvironmem;
        g_penvironmem = NULL;
    }

    //关闭日志文件
    if (g_ngx_log.fd != STDERR_FILENO && g_ngx_log.fd != -1)
    {
        close(g_ngx_log.fd);
        g_ngx_log.fd = -1;
    }
}