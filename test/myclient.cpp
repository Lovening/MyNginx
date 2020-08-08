#include <stdio.h>
#include <sys/socket.h> //socket AF_INET
#include <stdlib.h>     //exit
#include <unistd.h>     //close
#include <string.h>      //memset

#include <arpa/inet.h> //htons inet_pton


#pragma pack (1)
//包头
typedef struct _COMM_PKG_HEADER
{
	unsigned short pkgLen;    
	unsigned short msgCode;   
	int            crc32;     
}COMM_PKG_HEADER, *LPCOMM_PKG_HEADER;

//包体1
typedef struct _STRUCT_REGISTER
{
	int           iType;          //类型
	char          username[56];   //用户名 
	char          password[40];   //密码

}STRUCT_REGISTER, *LPSTRUCT_REGISTER;

//包体2
typedef struct _STRUCT_LOGIN
{
	char          username[56];   //用户名 
	char          password[40];   //密码

}STRUCT_LOGIN, *LPSTRUCT_LOGIN;


#pragma pack() 

int  g_ipkgheader_len = sizeof(COMM_PKG_HEADER);


int MySend(int fd, char *buf, int ibuflen)
{
    int isentlen = 0;
    int itempsend;

    while (isentlen < ibuflen)
    {
        itempsend = send(fd, buf + isentlen, ibuflen - isentlen, 0);
        if (itempsend == -1 || itempsend == 0)
        {
            return -1;
        }

        isentlen += itempsend;
    }

    return 0;
}

void SendData(int fd)
{
    //组包
    int iCnt = g_ipkgheader_len + sizeof(STRUCT_REGISTER);
    char *psend_buf = (char*)new char[iCnt];
    LPCOMM_PKG_HEADER pPkgHeader = LPCOMM_PKG_HEADER(psend_buf);
    pPkgHeader->msgCode = htons(1);
    pPkgHeader->pkgLen = htons(sizeof(psend_buf));
    pPkgHeader->crc32 = htonl(1234);

    LPSTRUCT_REGISTER pReg = (LPSTRUCT_REGISTER)(psend_buf + g_ipkgheader_len);
    pReg->iType = htonl(100);
    strcpy(pReg->username, "root");
    pReg += sizeof("root");
    strcpy(pReg->password, "liwenlang");

    if (-1 == MySend(fd, psend_buf, g_ipkgheader_len + sizeof(STRUCT_REGISTER)))
    {
        printf("MySend()调用失败！");
        delete[] psend_buf;
        return;
    }

    delete[] psend_buf;

    psend_buf =  new char[g_ipkgheader_len + sizeof(_STRUCT_LOGIN)];
    pPkgHeader = LPCOMM_PKG_HEADER(psend_buf);
    pPkgHeader->msgCode = htons(2);
    pPkgHeader->pkgLen = htons(sizeof(psend_buf));
    pPkgHeader->crc32 = htonl(5678);
    LPSTRUCT_LOGIN pLogin = LPSTRUCT_LOGIN(psend_buf + g_ipkgheader_len);
    strcpy(pLogin->username, "root");
    pReg += 50;
    strcpy(pLogin->password, "liwenlang");
    
    if (-1 == MySend(fd, psend_buf, g_ipkgheader_len + sizeof(STRUCT_LOGIN)))
    {
        printf("MySend()调用失败！");
        delete[] psend_buf;
        return;
    }

    delete[] psend_buf;

    printf("非常好，两个数据包通过SendData()都发送完毕");
}


int main()
{
    //创建socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);

    if (inet_pton(AF_INET, "192.168.1.121", &serv_addr.sin_addr) <= 0)
    {
        printf("调用inet_pton()失败!\n");
        close(fd);
        exit(1);
    }

    //连接socket
    if (connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))
     < 0)
    {
        printf("调用connect()失败！\n");
        close(fd);
        exit(1);

    }

    //设置选项
    // if (-1 == setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuseaddr, sizeof(int)))

    int iSendRecvTimeOut = 2;
    //if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,(const void*)&iSendRecvTimeOut, sizeof(int)) == -1 )
     struct timeval timeout={3,0};
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,&timeout, sizeof(timeout)) == -1 )
    {
        printf("调用setsockopt(SO_SNDTIMEO)失败！\n");
        close(fd);
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,&timeout, sizeof(timeout)) == -1 )
    //if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&iSendRecvTimeOut, sizeof(int)) == -1 )
    {
        printf("调用setsockopt(SO_RCVTIMEO)失败！\n");
        close(fd);
        exit(1);
    }

    printf("非常好，connect()成功！\n");

    //发送数据
    //while (1)
    {
        SendData(fd);
    }

    printf("这是一个客户端测试程序！\n");

    close (fd);
    return 0;
}

