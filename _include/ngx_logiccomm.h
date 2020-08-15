
#ifndef __NGX_LOGICCOMM_H__
#define __NGX_LOGICCOMM_H__

//结构定义------------------------------------
#pragma pack (1)

typedef struct _STRUCT_REGISTER
{
	int           iType;          //类型
	char          username[56];   //用户名 
	char          password[40];   //密码

}STRUCT_REGISTER, *LPSTRUCT_REGISTER;

typedef struct _STRUCT_LOGIN
{
	char          username[56];   //用户名 
	char          password[40];   //密码

}STRUCT_LOGIN, *LPSTRUCT_LOGIN;


#pragma pack() //取消指定对齐，恢复缺省对齐

#endif
