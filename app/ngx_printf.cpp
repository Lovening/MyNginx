//和打印格式相关的函数放这里

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>   //类型相关头文件

#include "ngx_global.h"
#include "ngx_macro.h"
#include "ngx_func.h"

static u_char *ngx_sprintf_num(u_char *buf, u_char *last, uint64_t ui64,u_char zero, uintptr_t hexadecimal, uintptr_t width);

//----------------------------------------------------------------------------------------------------------------------
//对于 nginx 自定义的数据结构进行标准格式化输出,就像 printf,vprintf 一样
u_char *ngx_slprintf(u_char *buf, u_char *last, const char *fmt, ...) 
{
    va_list   args;
    u_char   *p;

    va_start(args, fmt); 
    p = ngx_vslprintf(buf, last, fmt, args);
    va_end(args);       
}

//----------------------------------------------------------------------------------------------------------------------
//对于 nginx 自定义的数据结构进行标准格式化输出
//buf：往这里放数据
//last：放的数据不要超过这里
//fmt：以这个为首的一系列可变参数
//支持的格式： %d【%Xd/%xd】:数字,    %s:字符串      %f：浮点,  %P：pid_t
u_char *ngx_vslprintf(u_char *buf, u_char *last,const char *fmt,va_list args)
{
   uintptr_t width, sign, hex, frac_width, scale, n;

   u_char zero;     //前置0，空格
   uint64_t i64;    //%d
   uint64_t ui64;   //%ud
   u_char *p;       //%s
   double f;        //%f
   uint64_t frac;   //%.2f，小数点后

   while (*fmt && buf < last)
   {
       if (*fmt == '%')
       {
           //初始化
           zero = (u_char)(*(++fmt) == '0' ? '0' : ' ');
           width = 0;
           sign = 1;
           hex = 0;
           frac_width = 0;
           i64 = 0;
           ui64 = 0;

            //解析width
            while(*fmt >= '0' && *fmt <= '9')
            {
                width = width * 10 + (*fmt++ - '0');
            }
            
            //标记
            do
            {
                switch(*fmt)
                {
                case 'u':
                    sign = 0;
                    fmt++;
                    continue;
                 case 'X':
                    hex = 2;
                    sign = 0;
                    fmt++;
                    continue;
                case 'x':
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;
                 case '.':
                    fmt++;
                    while(*fmt >= '0' && *fmt <= '9')
                    {
                        frac_width = frac_width * 10 + (*fmt++ - '0');
                    }
                    break;
                 default:
                    break;
                }
            }while(false);

            switch(*fmt)
            {
            case '%':
                *buf++ = '%';
                continue;
            
            case 'd':
                if (sign)
                {
                    i64 = (int64_t) va_arg(args, int);
                }
                else
                {
                    ui64 = (uint64_t) va_arg(args, u_int);
                }
                break;

            case 's':
                p = va_arg(args, u_char*);

                while(*p && buf < last)
                {
                    *buf++ = *p++;
                }

                fmt++;
                continue;

            case 'P': //pid_t类型
                i64 = (int64_t) va_arg(args, pid_t);
                sign = 1;
                break;

            case 'f':
                f = va_arg(args, double);
                if (f < 0)
                {
                    *buf++ = '-';
                    f = -f;
                }
                
                ui64 = (int64_t) f;
                frac = 0;

                if (frac_width)
                {
                    scale = 1;  //放大倍数
                    for (n = frac_width; n; n--)
                    {
                        scale *= 10;
                    }
                }

                //取出小数部分
                frac = (uint64_t) ((f - (double)ui64)) * scale + 0.5;   //四舍五入
                if (frac == scale)
                {
                    ui64++;
                    frac = 0;
                }

                //整数部分
                buf = ngx_sprintf_num(buf, last, ui64, zero, 0, width);

                if (frac_width)
                {
                    if (buf < last)
                    {
                        *buf++ = '.';
                    }
                    buf = ngx_sprintf_num(buf, last, frac, '0', 0, frac_width);
                }
                fmt++;
                continue;


                default:
                *buf++ = *fmt++;
                continue;
            }

            //处理显示%d
            if (sign)
            {
                if (i64 < 0)
                {
                    *buf++ = '-';
                    ui64 = (uint64_t) -i64;
                }
                else
                {
                    ui64 = (uint64_t)i64;
                }
            }

            buf = ngx_sprintf_num(buf, last, ui64, zero, hex, width);
            fmt++;
         }
       else
       {
           *buf++ = *fmt++;
       }
   }

   return buf;
}

//----------------------------------------------------------------------------------------------------------------------
//以一个指定的宽度把一个数字显示在buf对应的内存中, 如果实际显示的数字位数 比指定的宽度要小 ,比如指定显示10位，而你实际要显示的只有“1234567”，那结果可能是会显示“   1234567”
    //【参数width=0】，则按实际宽度显示
    //%Xd以十六进制数字格式显示出来
//buf：往这里放数据
//last：放的数据不要超过这里
//ui64：显示的数字         
//zero:显示内容时，格式字符%后边接的是否是个'0',如果是zero = '0'，否则zero = ' ' 【一般显示的数字位数不足要求的，则用这个字符填充】，比如要显示10位，而实际只有7位，则后边填充3个这个字符；
//hexadecimal：是否显示成十六进制数字 0：不
//width:显示内容时，格式化字符%后接的如果是个数字比如%16，那么width=16，所以这个是希望显示的宽度值【如果实际显示的内容不够，则后头用0填充】
static u_char * ngx_sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero, uintptr_t hexadecimal, uintptr_t width)
{
   u_char *p, temp[NGX_INT64_LEN + 1];
   size_t len;
   uint32_t ui32;

   static u_char hex[] = "0123456789abcdef";
   static u_char HEX[] = "0123456789ABCDEF";

   p = temp + NGX_INT64_LEN;

   if (hexadecimal == 0)
   {
       if (ui64 <= (uint64_t) NGX_MAX_UINT32_VALUE)
       {
           ui32 = (uint32_t)ui64;

           do 
           {
               *--p = (ui32 % 10 + '0');
           }
           while(ui32 /= 10);

       }
       else
       {
           do
           {
               *--p = (u_char)(ui64 % 10 + '0');
           }
           while(ui64 /= 10);
       }
   }
   else if (hexadecimal == 1)
   {
       do
       {
           *--p = hex[(uint32_t)(ui64 & 0xf)];
       }
       while (ui64 >>= 4);
   }
   else 
   {
       do
       {
           *--p = HEX[(uint32_t)(ui64 & 0xf)];
       }
       while(ui64 >>= 4);
   }

    //根据p,计算
   len = (temp + NGX_INT64_LEN) - p;

   //填充
   while(len++ < width && buf < last)
   {
       *buf++ = zero;
   }

    //还原
   len = (temp + NGX_INT64_LEN) - p;
   if ((buf + len) >= last)
   {
       len = last - buf;
   }

   return ngx_cpymem(buf, p, len);
}   

