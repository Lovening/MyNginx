#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char const *argv[])
{
    printf("Hello Nginx\n ");
    
    for (;;)
    {
        sleep(1);
        printf("父进程休息一秒！\n");
    }

    printf("程序退出！");

    return 0;
}
