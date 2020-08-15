#ifndef _NGX_COMM_H_
#define _NGX_COMM_H_

#define _PKG_MAX_LENGTH		30000

//收包状态定义
#define _PKG_HD_INIT		0	//接收包头
#define _PKG_HD_RECVING 	1	//包头未接收完整，继续接收
#define _PKG_BD_INIT		2	//开始接收包体
#define _PKG_BD_RECVING		3	//继续接收包体

#define _DATA_BUFSIZE		20 //接收包头的数组字节数


#pragma pack(1)

//包头
typedef struct _COMM_PKG_HEADER
{
	unsigned short 	sPckLen;
	unsigned short 	sMsgCode;
	int				iCrc32;		//CRC32校验
}COMM_PKG_HEADER, *LPCOMM_PKG_HEADER;

#pragma pack()



#endif
