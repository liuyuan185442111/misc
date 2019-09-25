安装jdk
-
tomcat是一个java应用，所以需要先安装jdk。
centos 6.5自带openjdk，版本如下：
```bash
[root@zck ~]# java -version
java version "1.7.0_65"
OpenJDK Runtime Environment (rhel-2.5.1.2.el6_5-x86_64 u65-b17)
OpenJDK 64-Bit Server VM (build 24.65-b04, mixed mode)
```
进一步查看JDK
```bash
[root@zck ~]# rpm -qa | grep java
tzdata-java-2012c-1.el6.noarch
java-1.6.0-openjdk-1.6.0.0-1.45.1.11.1.el6.x86_64penJDK
```
卸载OpenJDK
```bash
[root@zck ~]# rpm -e --nodeps tzdata-java-2012c-1.el6.noarch
[root@zck ~]# rpm -e --nodeps java-1.6.0-openjdk-1.6.0.0-1.45.1.11.1.el6.x86_64
```
从Oracle官网下载rpm包，直接打开安装或用以下命令安装

	[root@zck ~]# rpm -ivh jdk-7-linux-x64.rpm

安装tomcat
-
去官网下载最新版
http://tomcat.apache.org/download-80.cgi
将文件上传到/usr/local目录中
执行解压缩
`tar zxvf apache-tomcat-8.0.8.tar.gz`
将目录apache-tomcat-8.0.8命名为tomcat
`mv apache-tomcat-8.0.8 tomcat`
运行/usr/local/tomcat/bin/startup.sh就会启动tomcat，运行shutdown.sh会停止tomcat。
修改默认端口
-
在tomcat安装目录的conf文件夹里有个server.xml文件，修改里面的

	<Connector port="8080"
这段代码，那个port值就是端口号。
<font color=red>**并注意配置Linux防火墙。**</font>
参考文献
-
在CentOS 6.3中安装与配置JDK-7
http://www.jb51.net/os/RedHat/73016.html
在CentOS 6.3中安装与配置Tomcat-7方法
http://www.jb51.net/os/RedHat/73032.html
在CentOS上安装tomcat
http://www.cnblogs.com/xsi640/p/3757015.html