#include <stdio.h>
#include <signal.h>
#include <unistd.h>
//#include <errno.h>

int g_mysign = 0;

void muNEfunc(int value)
{
	g_mysign = value;
}

void sig_usr(int sign)
{
	muNEfunc(22);
	
    if (sign == SIGUSR1)
    {
        printf("收到勒SIGUSR1信号！\n");
    }
    else if (sign == SIGUSR2)
    {
        printf("收到勒SIGUSR2信号！\n");
    }
    else
    {
        printf("收到未捕捉的信号%d!\n", sign);
    }

}


int main(int argc, char const *argv[])
{
    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
    {
        printf("无法不住SIGUSR1信号！\n");
    }

    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
    {
       printf("无法不住SIGUSR2信号！\n");
    }

    for (;;)
    {
        sleep(1);
        printf("休息1秒!\n");
		
		muNEfunc(15);
        printf("g_mysign=%d\n",g_mysign); 
        
    }
    
    printf("程序退出！\n");

    return 0;
}
