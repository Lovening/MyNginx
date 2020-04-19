
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void sig_quit(int sign)
{
    if (sign == SIGQUIT)
    {
        printf("收到勒SIGQUIT信号！\n");
    }
    if (signal(SIGQUIT, SIG_DFL) == SIG_ERR)
    {
        printf("SIGQUIT设置缺省处理失败！\n");
        exit(1);
    }
}

int main(int argc, char *const *argv)
{
    sigset_t newmask, oldmask;
    if (signal(SIGQUIT, sig_quit) == SIG_ERR)  //"ctrl+\"
    {
        printf("无法捕捉SIGQUIT信号！\n");
        exit(1);
    }

    sigemptyset(&oldmask);
    sigaddset(&oldmask, SIGINT);

    //设置初始信号集
    if (sigprocmask(SIG_BLOCK, &oldmask, NULL) < 0) 
    {
        printf("sigprocmask(SIG_SETMASK)调用失败！\n");
        exit(1);
    }

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGQUIT);
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) //屏蔽int
    {
        printf("sigprocmask(SIG_BLOCK)调用失败！\n");
        exit(1);
    }

    printf("我要休息十秒了，-----begin--, 此时我无法收到SIGQUIT信号！\n");
    sleep(10);
    printf("我已经休息十秒了，-----end--\n");

    if (sigismember(&oldmask, SIGQUIT))
    {
        printf("SIGQUIT信号被屏蔽了！\n");
    }
    else
    {
        printf("SIGQUIT信号未被屏蔽！\n");
    }

     if (sigismember(&oldmask, SIGINT))
    {
        printf("SIGINT信号被屏蔽了！\n");
    }
    else
    {
        printf("SIGINT信号未被屏蔽！\n");
    }

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
    {
        printf("sigprocmask(SIG_SETMASK)调用失败！\n");
        exit(1);
    }
    
    printf("sigprocmask(SIG_SETMASK)调用成功！\n");

    if (sigismember(&oldmask, SIGQUIT))
    {
        printf("SIGQUIT信号被屏蔽了！\n");
    }
    else
    {
        printf("SIGQUIT信号未被屏蔽！您可以发送SIGQUIT信号了，我要sleep(10)秒钟!!!!!!\n");
        int mysl = sleep(10);
        if (mysl > 0)
        {
            printf("sleep还没睡够，剩余%d秒", mysl);
        }
    }

    printf("程序退出！\n");

    return 0;
}

