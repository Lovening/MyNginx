#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

#define SERV_PORT 9000

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    
    if (inet_pton(AF_INET, "192.168.1.120", &serv_addr.sin_addr) <= 0)
    {
        printf("调用inet_pton()失败！\n");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("调用connect()失败！\n");
        exit(1);
    }

    int n;
    char recvline[1000 + 1];
    while ((n = read(sockfd, recvline, 1000)) > 0)
    {
        recvline[n] = 0;
        printf("收到的内容为： %s\n", recvline);
    }

    close(sockfd);
    printf("程序执行完毕，退出！\n");

    return 0;
}