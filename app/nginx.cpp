#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ngx_func.h"
#include "ngx_c_conf.h"

char  **g_ppargv = NULL;
char  *g_penvironmem = NULL; 
int   g_ienvironlen = 0; 

pid_t g_ngx_pid;               //当前进程的pid

void freeresource();

int main(int argc, char* const argv[])
{
    printf("Hello MyNginx!\n");
    int iexitcode = 0;

    g_ppargv = (char**)argv;
    g_ngx_pid = getpid();

    //读取配置文件
    CConfig* pConfig = CConfig::GetInstance();
    if (!pConfig->Load("nginx.conf"))
    {
        ngx_log_stderr(0, "配置文件[%s]载入失败，退出！", "nginx.conf");
        iexitcode = 2;
        goto lblexit;
        //exit(1);
    }

    //日志初始化
    ngx_log_init();

  //移走环境变量
    ngx_init_setproctitle();
    //设置标题
    ngx_setproctitle("nginx: master process");

    // int port = pConfig->GetIntDefault("ListenPort", 0);
    // printf("port = %d\n", port);

    // const char *pChar = pConfig->GetString("DBInfo");
    // if (NULL != pChar)
    //     printf("DBInfo = %s\n", pChar);


    //strncpy(argv[0], "22", 1);
    //strcpy(argv[0], "33");

    // printf("argv[0]=%s\n", argv[0]);
    // printf("argv[1]=%s,地址是%x\n", argv[1], &argv[1]);
    // printf("env[0]=%s,地址是%x\n", environ[0], &environ[0]);
    // printf("env[1]=%s,地址是%x\n", environ[1], &environ[1]);
    // printf("%d\n", sizeof(int *));

    //日志打印
    //ngx_log_stderr(0, "%s=%d", "logcode", 14);
    //ngx_log_error_core(5, 8, "sssss核心显示");

    for (;;)
    {
        printf("休息一秒！\n");
        sleep(1);
    }

lblexit:
    freeresource();
    printf("MyNginx Bye!");
    return iexitcode;
}


void freeresource()
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