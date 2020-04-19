#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char const *argv[])
{
    printf("Hello Nginx\n ");

    pid_t pid;
    pid = fork();

    //signal(SIGHUP, SIG_IGN);

    if (pid < 0)
    {

    }
    else if (pid == 0)
    {
        setsid();
        for (;;)
            {
                sleep(1);
                printf("子进程休息一秒！\n");
            }
    }
    else
    {
        for (;;)
        {
            sleep(1);
            printf("父进程休息一秒！\n");
        }
    }

    printf("程序退出！");

    return 0;
}
