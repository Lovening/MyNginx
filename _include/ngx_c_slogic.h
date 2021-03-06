﻿
#ifndef __NGX_C_SLOGIC_H__
#define __NGX_C_SLOGIC_H__

#include <sys/socket.h>
#include "ngx_c_socket.h"


//处理逻辑和通讯的子类
class CLogicSocket : public CSocket   
{
public:
	CLogicSocket();                                                        
	virtual ~CLogicSocket();                                            
	virtual bool Initialize();                                           

public:
	//各种业务逻辑相关函数都在之类
	bool HandleRegister(lpngx_connection_t pConn,LPSTRUCT_MSG_HEADER pMsgHeader,char *pPkgBody,unsigned short iBodyLength);
	bool HandleLogIn(lpngx_connection_t pConn,LPSTRUCT_MSG_HEADER pMsgHeader,char *pPkgBody,unsigned short iBodyLength);

public:
	virtual void threadRecvProcFunc(char *pMsgBuf);
};

#endif
