//和信号有关的函数放这里
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>    //信号相关头文件 
#include <errno.h>     //errno
#include <sys/wait.h>  //waitpid

#include "ngx_macro.h"
#include "ngx_func.h" 
#include "ngx_global.h"

typedef struct
{
    int signo;
    const char *signame;
    void (*handler)(int signo, siginfo_t* siginfo, void *ucontext);   //函数指针
}ngx_signal_t;

//信号处理函数
static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);
static void ngx_process_get_status(void);

ngx_signal_t signals[] = {
                        {SIGHUP, "SIGHUP", ngx_signal_handler},//连接断开,xshell标识1
                        {SIGINT, "SIGINT", ngx_signal_handler},//终端断开-标识2
                        {SIGTERM, "SIGTERM", ngx_signal_handler},//15
                        {SIGCHLD, "SIGCHLD", ngx_signal_handler}, //子进程改变 17
                        {SIGQUIT, "SIGQUIT", ngx_signal_handler},//终端退出 3
                        {SIGIO, "SIGIO", ngx_signal_handler}, //异步IO 
                        {SIGSYS, "SIGSYS, SIG_IGN", NULL},  //31

                        {0, NULL, NULL}   //忽略该信号                 
                    };



 //初始化信号函数，用于注册信号处理程序
int ngx_init_signals()
{
    ngx_signal_t *sig;
    struct sigaction sa; 

    for (sig = signals; sig->signo != 0; ++sig)
    {
        memset(&sa, 0, sizeof(struct sigaction));
        if (NULL != sig->handler)
        {
            sa.sa_sigaction = sig->handler;
            sa.sa_flags = SA_SIGINFO;
        }
        else
        {
            sa.sa_handler = SIG_IGN;
        }

        sigemptyset(&sa.sa_mask);

        if (-1 == sigaction(sig->signo, &sa, NULL))
        {
            ngx_log_error_core(NGX_LOG_EMERG, errno, "sigaction(%s) failed", sig->signame);
            return -1;
        }
        else
        {
            //ngx_log_stderr(0, "sigaction(%s) success!", sig->signame);
        }
    }

    return 0;
}


static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext)
{
    //printf("来信号了！");

    ngx_signal_t *sig;
    char *action;

    for (sig = signals; sig->signo != 0; ++sig)
    {
        if (sig->signo == signo)
        {
            break;
        }
    }

    action = (char*)"";

    if (NGX_PROCESS_MASTER == g_iprocesstype)
    {
        switch (signo)
        {
        case SIGCHLD:
            g_ngx_reap = 1;
            break;
        default:
            break;
        }
    }
    else if (NGX_PROCESS_WORKER == g_iprocesstype)
    {

    }
    else
    {

    }

    //记录日志
    if(siginfo && siginfo->si_pid)
    {
        ngx_log_error_core(NGX_LOG_NOTICE,0,"signal %d (%s) received from %P%s", signo, sig->signame, siginfo->si_pid, action); 
    }
    else
    {
        ngx_log_error_core(NGX_LOG_NOTICE,0,"signal %d (%s) received %s",signo, sig->signame, action);//没有发送该信号的进程id，所以不显示发送该信号的进程id
    }
    
    if (signo == SIGCHLD)
    {
        ngx_process_get_status();
    }
      
}           


//获取子进程的结束状态，防止单独kill子进程时子进程变成僵尸进程
static void ngx_process_get_status(void)
{
   pid_t pid;
   int status;
   int err;
   int one = 0;

   for (;;)
   {
       //waitpid获取子进程的终止状态，这样，子进程就不会成为僵尸进程了；
         //第一个参数为-1，表示等待任何子进程，
        //第二个参数：保存子进程的状态信息(大家如果想详细了解，可以百度一下)。
        //第三个参数：提供额外选项，WNOHANG表示不要阻塞，让这个waitpid()立即返回
        pid = waitpid(-1, &status, WNOHANG);        

        if(pid == 0) 
        {
            return;
        } 

        if(pid == -1)//waitpid调用有错误
        {
            err = errno;
            if(err == EINTR)           //调用被某个信号中断
            {
                continue;
            }

            if(err == ECHILD  && one)  //没有子进程
            {
                return;
            }

            if (err == ECHILD)         //没有子进程
            {
                ngx_log_error_core(NGX_LOG_INFO,err,"waitpid() failed!");
                return;
            }
            ngx_log_error_core(NGX_LOG_ALERT,err,"waitpid() failed!");
            return;
        }  
        //-------------------------------
        one = 1;  //标记waitpid()返回了正常的返回值
        if(WTERMSIG(status))  //获取使子进程终止的信号编号
        {
            ngx_log_error_core(NGX_LOG_ALERT,0,"pid = %P exited on signal %d!",pid,WTERMSIG(status)); //获取使子进程终止的信号编号
        }
        else
        {
            ngx_log_error_core(NGX_LOG_NOTICE,0,"pid = %P exited with code %d!",pid,WEXITSTATUS(status)); //WEXITSTATUS()获取子进程传递给exit或者_exit参数的低八位
        }
   }
}
