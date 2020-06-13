#ifndef __NGX_FUNC_H__
#define __NGX_FUNC_H__

#define  MYVER  "1.2"


//字符串相关函数
void Rtrim(char *string);
void Ltrim(char *string);

//标题初始化
void ngx_init_setproctitle();

//设置可执行程序标题
void ngx_setproctitle(const char *title);

#endif  