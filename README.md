# MyNginx
Nginx实战

参考TortoiseGit:   https://blog.csdn.net/h0422856/article/details/51276093

//遇到的问题及解决方法：

1./usr/bin/ld: cannot open output file test: Permission denied

方法1）修改用户组，chown
方法2)用root权限登入，
Xshell使用root用户连接Linux,修改配置文件--------------
https://blog.csdn.net/Zhang_0507/article/details/86608031?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase

2.undefined reference to 连接错误
1）全局变量， extern int g_xxxx 只是声明，必须定义： int g_xxxx
https://blog.csdn.net/qq_38880380/article/details/81474580
2）makefil未添加编译文件

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



