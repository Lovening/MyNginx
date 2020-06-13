#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ngx_func.h"
#include "ngx_c_conf.h"

char  **g_ppargv = NULL;
char  *g_penvironmem = NULL; 
int   g_ienvironlen = 0; 

int main(int argc, char* const argv[])
{
    printf("Hello MyNginx!\n");
    g_ppargv = (char**)argv;

    //移走环境变量
    ngx_init_setproctitle();

    //读取配置文件
    CConfig* pConfig = CConfig::GetInstance();
    if (!pConfig->Load("nginx.conf"))
    {
        printf("配置文件载入失败，退出！\n");
        exit(1);
    }

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

    for (;;)
    {
        printf("休息一秒！\n");
        sleep(1);
    }

    if (g_penvironmem)
    {
        delete[] g_penvironmem;
        g_penvironmem = NULL;
    }

    return 0;
}
