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

//和日志，打印输出有关
void ngx_log_init();
void ngx_log_stderr(int err, const char *fmt, ...);
void ngx_log_error_core(int level,  int err, const char *fmt, ...);

u_char *ngx_log_errno(u_char *buf, u_char *last, int err);
u_char *ngx_slprintf(u_char *buf, u_char *last, const char *fmt, ...);
u_char *ngx_vslprintf(u_char *buf, u_char *last,const char *fmt,va_list args);


#endif  