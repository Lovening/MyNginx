#include <stdio.h>
#include <stdlib.h>
//#include <sys/stat.h>
//#include <signal.h>
#include <fcntl.h> //O_RDWR
#include <unistd.h>  //STDERR_FILENO

int main(int argc, char const *argv[])
{
    pid_t pid = fork();
    switch (pid)    
    {
    case -1:
        return -1; //返回-1，写日志
        break;
    case 0:
        break;
    default:
        exit(0);
        break;
    }
    
    //子进程走下来
    if (setsid() == -1)
    {
        return -1;
    }

    //限制文件权限
    umask(0);

    //重定向至空设备
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);

    if (fd > STDERR_FILENO)
    {
        close(fd);

        while(1)
        {
            sleep(1);
            printf("正在休息1秒!\n");
        }
    }
    else
    {
        return -1;
    }

    return 0;
}
