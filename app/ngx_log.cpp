//和日志相关的函数放之类

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>    //uintptr_t
#include <stdarg.h>    //va_start....
#include <unistd.h>    //STDERR_FILENO等
#include <sys/time.h>  //gettimeofday
#include <time.h>      //localtime_r
#include <fcntl.h>     //open
#include <errno.h>     //errno

#include "ngx_global.h"
#include "ngx_macro.h"
#include "ngx_func.h"
#include "ngx_c_conf.h"

//全局量---------------------
//错误等级，和ngx_macro.h里定义的日志等级宏是一一对应关系
static u_char err_levels[][20]  = 
{
    {"stderr"},    //0：控制台错误
    {"emerg"},     //1：紧急
    {"alert"},     //2：警戒
    {"crit"},      //3：严重
    {"error"},     //4：错误
    {"warn"},      //5：警告
    {"notice"},    //6：注意
    {"info"},      //7：信息
    {"debug"}      //8：调试
};

ngx_log_t   g_ngx_log;


//----------------------------------------------------------------------------------------------------------------------
//描述：通过可变参数组合出字符串【支持...省略号形参】，自动往字符串最末尾增加换行符【所以调用者不用加\n】， 往标准错误上输出这个字符串；
//     如果err不为0，表示有错误，会将该错误编号以及对应的错误信息一并放到组合出的字符串中一起显示；
void ngx_log_stderr(int err, const char *fmt, ...)
{    
    va_list args;
    u_char *p, *last;

    u_char errstr[NGX_MAX_ERROR_STR + 1];
    memset(errstr, 0, sizeof(errstr));

    last = errstr + NGX_MAX_ERROR_STR;
    p = ngx_cpymem(errstr, "nginx: ", 7);

    va_start(args, fmt);
    p = ngx_vslprintf(p, last, fmt, args);
    va_end(args);

    if (err)
    {
        p = ngx_log_errno(p, last, err);
    }

    if (p >= (last - 1))
    {
        p = last - 2;
    }

    *p++ = '\n';

    //写标准错误信息
    write(STDERR_FILENO, errstr, p - errstr);

}

//----------------------------------------------------------------------------------------------------------------------
//错误编号对应的错误字符串，保存到buffer中
u_char *ngx_log_errno(u_char *buf, u_char *last, int err)
{   
    char *perrorinfo = strerror(err);
    size_t len = strlen(perrorinfo);

    char leftstr[10] = {0};
    sprintf(leftstr, " (%d: ", err);
    size_t leftlen = strlen(leftstr);

    char rightstr[] = ") ";
    size_t rightlen = strlen(rightstr);
    
    if (buf + len + leftlen + rightlen < last)
    {
        buf = ngx_cpymem(buf, leftstr, leftlen);
        buf = ngx_cpymem(buf, perrorinfo, len);
        buf = ngx_cpymem(buf, rightstr, rightlen);
    }

    return buf;
}

//----------------------------------------------------------------------------------------------------------------------
//往日志文件中写日志
//    定向为标准错误，则直接往屏幕上写日志【比如日志文件打不开，则会直接定位到标准错误，此时日志就打印到屏幕上，参考ngx_log_init()】
//level:一个等级数字，我们把日志分成一些等级，以方便管理、显示、过滤等等，如果这个等级数字比配置文件中的等级数字"LogLevel"大，那么该条信息不被写到日志文件中
//err：是个错误代码，如果不是0，就应该转换成显示对应的错误信息,一起写到日志文件中，
void ngx_log_error_core(int level,  int err, const char *fmt, ...)
{
    //格式2020/06/14 12:42:14 [warn] 0: sssss核心显示 (8: Exec format error)   
    u_char *last;
    u_char errstr[NGX_MAX_ERROR_STR + 1];

    memset(errstr, 0, sizeof(errstr));
    last = errstr + NGX_MAX_ERROR_STR;

    struct timeval tv;
    struct tm tm;
    time_t sec;
    u_char *p = NULL;
    va_list args;

    memset(&tv, 0, sizeof(struct timeval));
    memset(&tm, 0, sizeof(struct tm));

    //获取当前时间
    gettimeofday(&tv, NULL);  
    sec = tv.tv_sec;
    localtime_r(&sec, &tm);
    tm.tm_mon++;
    tm.tm_year += 1900;

    u_char strcurtime[40] = {0};
    ngx_slprintf(strcurtime, (u_char*)-1, "%4d/%02d/%02d %02d:%02d:%02d ",
                    tm.tm_year, tm.tm_mon, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec);
    p = ngx_cpymem(errstr, strcurtime, strlen((const char*)strcurtime));
    p = ngx_slprintf(p, last, "[%s] ", err_levels[level]);
    p = ngx_slprintf(p, last, "%P: ", g_ngx_pid);

    va_start(args, fmt);
    p = ngx_vslprintf(p, last, fmt, args);
    va_end(args);

    //打印错误代码
    if (err)
    {
        p = ngx_log_errno(p, last, err);
    }

    if (p >= last - 1)
    {
        p = (last - 1) - 1;
    }
    *p++ = '\n';

    //写文件
    ssize_t n;
    do
    {
        if (level > g_ngx_log.log_level) //日志等级太低
            break; 

        n = write(g_ngx_log.fd, errstr, p - errstr);
        if (-1 == n)
        {
            if (errno == ENOSPC)
            {
                //磁盘不足
            }
            else
            {
                if (g_ngx_log.fd != STDERR_FILENO)
                {
                    n = write(STDERR_FILENO, errstr, p - errstr);
                }
            }
        }

    }while(false);


}

//----------------------------------------------------------------------------------------------------------------------
//描述：日志初始化，就是把日志文件打开
void ngx_log_init()
{
    CConfig *pconfig = CConfig::GetInstance();
    const char *plogname = pconfig->GetString("Log");
    if (NULL == plogname)
    {
        plogname = (const char *) NGX_ERROR_LOG_PATH;
    }

    g_ngx_log.log_level = pconfig->GetIntDefault("LogLevel", NGX_LOG_NOTICE);

    //只写打开|追加到末尾|文件不存在则创建, mode = 0644：
    //文件访问权限， 6: 110    , 4: 100：     【用户：读写， 用户所在组：读，其他：读】
    g_ngx_log.fd = open(plogname, O_WRONLY|O_APPEND|O_CREAT, 0664);
    if (-1 == g_ngx_log.fd) //错误则定位置标准错误
    {
        ngx_log_stderr(errno, "[alert] could not open error log file: open() \"%s\" failed", plogname);
        g_ngx_log.fd = STDERR_FILENO;
    }
}

