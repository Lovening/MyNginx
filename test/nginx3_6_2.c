#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int g_myvalue = 0;

void sig_usr(int sign)
{
    if (sign == SIGCHLD)
    {
        printf("收到了SIGCHLD信号, 进程id=%d!\n", getpid());
        int status;
        waitpid(-1, &status, WNOHANG);
    }
}

int main(int argc, char const *argv[])
{
    printf("程序开始执行！\n");

    // pid_t pid;
    //pid = fork();

    //一个和fork()执行有关的逻辑判断（短路求值）
    //(fork() && fork()) || (fork() && fork());
    printf("每个实际用户ID的最大进程数=%ld\n", sysconf(_SC_CHILD_MAX));

    printf("程序结束执行！\n");

    // //拦截SIGCHLD信号
    // if (signal(SIGCHLD, sig_usr) == SIG_ERR)
    // {
    //     printf("signal()调用错误\n");
    //     exit(1);
    // }

    // if (pid < 0)
    // {
    //     printf("fork进程失败！\n");
    //     exit(1);
    // }
    // else if (pid > 0)
    // {
    //     while(1)
    //     {
    //         sleep(5);
    //         g_myvalue++;
    //         printf("父进程执行！值为%d\n", g_myvalue);
    //     }
         
    // }
    // else 
    // {
    //     while(1)
    //     {
    //         sleep(1);
    //         g_myvalue++;
    //         printf("子进程执行！值为%d\n", g_myvalue);
    //     }
    // }

    for(;;)
    {
        sleep(1);
        printf("休息1秒，进程id=%d\n", getpid());
    }


    return 0;
}
