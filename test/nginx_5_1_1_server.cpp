
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SERV_PORT 9000

int main()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    //设置ip,端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int result;
    result = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (-1 == result)
    {
        char *perrorinfo = strerror(errno);
        printf("bind返回值为%d, 错误码为：%d, 错误信息为：%s;\n", result, errno, perrorinfo);
        return -1;
    }

    result = listen(listenfd, 32);
    if (-1 == result)
    {
        char *perrorinfo = strerror(errno);
        printf("listen返回值为%d, 错误码为：%d, 错误信息为：%s;\n", result, errno, perrorinfo);
        return -1;
    }

    int connfd;
    const char* pcontent = "I sent sth to client!";

    for (;;)
    {
        //新的套接字
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        
       // char recvline[1000 + 1]; 
       // read(connfd,recvline,1000);  //等待客户端发送数据过来
        printf("本服务器给客户端发送了一串字符~~~~~~~~~~~!\n");

        write(connfd, pcontent, strlen(pcontent));
        close(connfd);
    }

    close(listenfd);
    
    return 0;
}