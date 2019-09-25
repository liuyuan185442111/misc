在线安装
-
```bash
yum install mysql-server mysql mysql-deve
service mysqld start # 启动服务
chkconfig mysqld on # 开机启动
netstat -anp | grep 3306 # 查看3306端口是否开始监听
mysqladmin -uroot password 'root' # 给root账号设置密码为root
mysql -uroot -p # 登陆
```
	mysqladmin -uroot -poldpassword newpassword
输入这个命令后，需要输入root的原密码，然后root的密码将改为newpassword。把命令里的root改为你的用户名，你就可以改你自己的密码了。
查看默认数据库
-
	SHOW databases;
查看已有的数据库，默认有information_schema、mysql、test三个数据库。test是空的，用来测试，删除即可；mysql用于系统管理；information_schema提供了访问数据库元数据的方式。
元数据是关于数据的数据，如数据库名或表名，列的数据类型，或访问权限等。在MySQL中，把information_schema看作是一个数据库，确切说是信息数据库。其中保存着关于MySQL服务器所维护的所有其他数据库的信息。如数据库名，数据库的表，表栏的数据类型与访问权限等。在INFORMATION_SCHEMA中，有数个只读表。它们实际上是视图，而不是基本表，因此，你将无法看到与之相关的任何文件。
修改MySQL的默认字符集
-
1. 查看字符集
`show variables like 'char%';`
2. 修改默认字符集
vim /etc/my.cnf
在[mysqld]部分最后添加
default-charactaracter-set=utf8
default-collation=utf8_general_ci
3. 重启mysql
`service mysqld restart`
忘记密码
-
1. kill掉mysql
找到mysql的pid，给mysql的pid发送TERM信号，让它结束运行：kill -TERM PID，或者直接强制关掉：kill -9 PID
2. 用`--skip_grant_tables`选项启动mysql，此时mysql将不使用它的权限表进行身份验证
3. 执行`Flush privileges;`使权限表读入内存并生效，以阻止其他可能的连接
4. 修改root口令
5. 重启mysql