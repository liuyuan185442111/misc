First of all：
vncserver在调用的时候，会根据你的配置来启用server端的监听端口，端口默认是从5900开始，再加上你的桌面号。比如你的桌面号为1，则vnc的连接端口号为5900+1=5901，比如你的桌面号为10000，则vnc的连接端口号为5900+10000=15900。

安装
-
	yum install tigervnc-server
配置
-
	vi /etc/sysconfig/vncservers
只需要两类内容就可以了，一个是定义用户，一个是定义用户登录情况：
```
VNCSERVERS="1:root 2:river"
VNCSERVERARGS[1]="-geometry 800x600 -nolisten tcp"
VNCSERVERARGS[2]="-geometry 800x600 -nolisten tcp"
```
接下来分别使用命令行模式先后登录root和river用户，登录后设定vncserver密码，登录后的操作如下：

	vncpasswd
	password输入密码
	verify再次输入密码
启动vnc服务
-
	service vncserver start
设置防火墙
-
	iptables -I INPUT -p tcp --dport 5901:5902 -j ACCEPT
	iptables -I INPUT -p udp --dport 5901:5902 -j ACCEPT
设置开机启动
-
	chkconfig vncserver on
	chkconfig --list vncserver
	vncserver 0:off 1:off 2:on 3:on 4:on 5:on 6:off
下载window客户端
-
从参考文献下载vncviewer，如下：
![vncviewer](http://img.blog.csdn.net/20150206182858679)
 
参考文献
-
CENTOS安装vnc【再整理】
http://xiaoaolijiangjiang.blog.163.com/blog/static/2418312020130230755787/
vncviewer.exe下载
http://www.cr173.com/soft/14314.html
