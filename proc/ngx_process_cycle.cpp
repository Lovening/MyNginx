//和开启子进程相关

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>   //信号相关头文件 
#include <errno.h>    //errno

#include "ngx_func.h"
#include "ngx_macro.h"
#include "ngx_c_conf.h"

static void ngx_start_worker_processes(int threadnums);
static int ngx_spawn_process(int threadnums,const char *pprocname);
static void ngx_worker_process_cycle(int inum,const char *pprocname);
static void ngx_worker_process_init(int inum);

static u_char  master_process[] = "master process";

//创建worker子进程
void ngx_master_process_cycle()
{    
    sigset_t set;
    sigemptyset(&set);

    //阻塞以下信号
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGWINCH);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1)
    {
        ngx_log_error_core(NGX_LOG_ALERT, errno, "ngx_master_process_cycle()中sigprocmask()执行失败！");
    }
    
    //设置标题
    size_t size = sizeof(master_process);
    size += g_iargvmemlen;
    if (size < 1000)
    {
        char title[1000] = {0};
        strcpy(title, (const char *) master_process);
        strcat(title, " ");
        for (int i = 0; i < g_iargc; ++i)
        {
            strcat(title, g_ppargv[i]);
        }

        ngx_setproctitle(title);
        ngx_log_error_core(NGX_LOG_NOTICE,0,"%s %P 启动并开始运行......!",title, g_ngx_pid);
    }

    //创建子进程
    CConfig *pconfig = CConfig::GetInstance();
    int iWorkerProcesses = pconfig->GetIntDefault("WorkerProcesses", 1);
    ngx_start_worker_processes(iWorkerProcesses);

    sigemptyset(&set);

    for ( ;; ) 
    {
        //ngx_log_error_core(0,0,"haha--这是父进程，pid为%P",g_ngx_pid);
        //printf("haha--这是父进程\n");

        //阻塞在这里，等待一个信号，此时进程是挂起的，不占用cpu时间，只有收到信号才会被唤醒（返回）；
        sigsuspend(&set);

        //printf("执行到sigsuspend下来了\n");

    }
}

//根据给定的参数创建指定数量的子进程d
static void ngx_start_worker_processes(int threadnums)
{
    for (int i = 0; i < threadnums; ++i)
    {
        ngx_spawn_process(i, "worker process");
    }
}

//描述：产生一个子进程
static int ngx_spawn_process(int inum, const char *pprocname)
{
   pid_t pid = fork();
   switch(pid)
   {
    case -1:
        ngx_log_error_core(NGX_LOG_ALERT, errno,"ngx_spawn_process()中fork()失败！");
        return -1;
        break;
    case 0:
        g_ngx_ppid = g_ngx_pid;
        g_ngx_pid = getpid();
        ngx_worker_process_cycle(inum, pprocname);
        break;
    default:
        break;

   }

   return pid;
}

//描述：worker子进程的功能函数，每个woker子进程，就在这里循环着了（无限循环【处理网络事件和定时器事件以对外提供web服务】）
static void ngx_worker_process_cycle(int inum,const char *pprocname) 
{
    g_iprocesstype = NGX_PROCESS_WORKER;

    ngx_worker_process_init(inum);
    ngx_setproctitle(pprocname);
    ngx_log_error_core(NGX_LOG_NOTICE,0,"%s %P 启动并开始运行......!", pprocname, g_ngx_pid); 

    //(stdout, NULL, _IONBF, 0);

    for (;;)
    {
        //ngx_log_error_core(0,0,"good--这是子进程，编号为%d,pid为%P！",inum, g_ngx_pid);
        //printf("good--这是子进程，编号为%d",inum);
        //fflush(stdout);
        //sleep(1);

        ngx_process_events_and_timers();
    }
}

//描述：子进程创建时调用本函数进行一些初始化工作
static void ngx_worker_process_init(int inum)
{
    sigset_t set;     
    sigemptyset(&set); 
    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1)  //原来是屏蔽那10个信号【防止fork()期间收到信号导致混乱】，现在不再屏蔽任何信号【接收任何信号】
    {
        ngx_log_error_core(NGX_LOG_ALERT,errno,"worker子进程编号为%d的ngx_worker_process_init()中sigprocmask()失败!", inum);
    }

    g_socket.ngx_epoll_init();
 
}
