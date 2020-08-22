# MyNginx
Nginx实战

参考TortoiseGit:   https://blog.csdn.net/h0422856/article/details/51276093
本地创建分支，删除，合并 https://www.cnblogs.com/hbujt/p/5554038.html

//遇到的问题及解决方法：

1./usr/bin/ld: cannot open output file test: Permission denied

方法1）修改用户组，chown
方法2)用root权限登入，
Xshell使用root用户连接Linux,修改配置文件--------------
https://blog.csdn.net/Zhang_0507/article/details/86608031?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase

2.undefined reference to 连接错误
1）全局变量， extern int g_xxxx 只是声明，必须定义： int g_xxxx
https://blog.csdn.net/qq_38880380/article/details/81474580
2）makefile未添加编译文件
3)class CSocket g_socekt;  缺少class类型定义

3.Clock skew detected.  Your build may be incomplete
修改系统时间
方法1）
timedatectl list-timezones |grep Shanghai    #查找中国时区的完整名称
timedatectl set-timezone Asia/Shanghai    #其他时区以此类推
https://www.cnblogs.com/jiu0821/p/6768473.html
date    查询系统时间
date -s 1/31/2012   命令可以修改系统日期
date -s 22:34:50    命令可以修改系统时间
方法2）
tzselect 产生beijing时间文件
sudo cp /usr/share/zoneinfo/Asia/Shanghai /etc/loacltime 复制文件，持久化


4.执行make，未生成.d目录
定位到对应代码行，查看是否是变量名写错，

5.linux系统忘记root密码
https://www.cnblogs.com/leonchan/p/10979735.html
CENTOS,Linux处理方式不同

6.makefile中常见的错误—missing separator. Stop.

A:https://blog.csdn.net/dumgeewang/article/details/7709412?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-2.channel_param&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-2.channel_param
命令行前缺少tab， 一定是makefile格式出问题了


7.new’未声明(在此函数内第一次使用),对‘operator new[](unsigned long)’未定义的引用,对‘operator delete[](void*)’未定义的引用,
A:gcc编译导致，换成g++编译通过

8.note: C++11 ‘constexpr’ only available with -std=c++11 or -std=gnu++11
A:添加-std=c++11选项

9.does not name a type
A:1、没有加调用函数的头文件
2、不存在xxx命名空间
3、包含头文件，但是调用的时候，类名写错了

10.链接错误：undefined reference to `pthread_create'
            undefined reference to `pthread_join'
A: 添加链接依赖      $(CC) -o $@ $^ -lpthread

          

