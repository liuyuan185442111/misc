在Linux中普通用户可以用passwd来修改自己的密码，可以用crontab来添加定时任务，但是它们的属性：

	-rwsr-xr-x. 1 root root 27832 Jun 10  2014 /bin/passwd
	-rwsr-xr-x 1 root root 57552 Mar 31  2016 /bin/crontab
	（各字段分别为：访问权限，文件硬链接数或目录子目录数，拥有者，拥有组，
	大小(以字节为单位)，最后修改时间，文件名）
	(那个点表示文件带有"SELinux的安全上下文")

这是通过SUID来实现的，如果一个文件被设置了SUID或SGID位，会分别表现在所有者或同组用户的权限的可执行位上。例如：

	-rwsr-xr-x 表示SUID被设置，且可执行位也被设置
	-rwSr--r-- 表示SUID被设置，但所有者权限中可执行位没有被设置
	-rwxr-sr-x 表示SGID被设置，且可执行位也被设置
	-rw-r-Sr-- 表示SGID被设置，但同组用户权限中可执行位没有被设置
在UNIX的实现中，文件权限用12个二进制位表示：

	11 10 9 8 7 6 5 4 3 2 1 0
	S  G  T r w x r w x r w x
第11位为SUID位，第10位为SGID位，第9位为sticky位，第8-0位对应于上面的三组rwx位。

对于一个进程来说，有3个相关的用户id：real UID，effective UID，saved set-user-ID。组ID也是类似，内核主要是根据euid和egid来确定进程对资源的访问权限。
**对程序文件调用exec()时，进程的real UID不变，如果程序文件的set-user-ID位打开了，进程的effective UID将设置为程序文件的所有者的ID，如果程序文件的set-user-ID位没有打开，进程的effective UID将维持不变，进程的saved set-user-ID将从effective UID复制。**
SUID让本来没有相应权限的用户运行这个程序时，可以访问他没有权限访问的资源。

SUID对目录没有影响。如果一个目录设置了SGID位，那么如果任何一个用户对这个目录有写权限的话，他在这个目录所建立的文件的组都会自动转为这个目录的所属组，而文件所有者不变，还是属于建立这个文件的用户。

上面还出现了stickybit粘滞位，普通文件的sticky位会被linux内核忽略，目录的sticky位表示这个目录里的文件只能被owner或root删除或移动。 比较典型的例子就是“/tmp”、“/var/tmp”目录。这两个目录作为Linux系统的临时文件夹，权限为“rwxrwxrwt”，即允许任意用户、任意程序在该目录中进行创建、删除、移动等操作，但普通用户不能删除或移动其他用户的文件。

u+s g+s o+t分别可以添加SUID，GUID，stickybit。

获取和设置euid的典型函数：

	uid_t getuid(void);
	uid_t geteuid(void);
	int setuid(uid_t uid);
	int seteuid(uid_t euid);

getuid() returns the real user ID of the calling process.
geteuid() returns the effective user ID of the calling process.
setuid() sets the effective user ID of the calling process. If the effective UID of the caller is root, the real UID and saved set-user-ID are also set.
seteuid() sets the effective user ID of the calling process. Unprivileged user processes may only set the effective user ID to the real user ID, the effective user ID or the saved set-user-ID.
对于非特权用户进程，setuid()和seteuid()作用相同，且只能将the effective user ID设置为the real user ID, the effective user ID or the saved set-user-ID，否则将errno设置为EPERM，并返回-1。
对于特权用户进程，setuid()还会设置the real UID and saved set-user-ID。

还有另外一些相关函数：
       int setreuid(uid_t ruid, uid_t euid);
       int setresuid(uid_t ruid, uid_t euid, uid_t suid);
       int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
但不同标准的实现不完全相同，不再细述。


来看一个例子：
```
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
int main()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tv.tv_sec += 3600;
    printf("settimeofday return %d",settimeofday(&tv, &tz));
    printf(", errno is %d, EPERM is %d\n", errno, EPERM);
    return 0;
}
```
root编译并u+s后，普通用户执行就会将系统时间加1个小时。

如果想让普通用户也可以修改系统时间可以用上面的方式，但如果能用date命令的话就简单多了，然而直接将调用settimeofday替换为调用date命令是不行的，**调用system时，如果effective UID与real UID不匹配，bash会将effective UID设置为real UID**，注意到在setdate中进程是有root权限的，只要通过setuid将real UID设置成和effective UID一样的值，调用date命令时就有了root权限。
```
//setdate.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		puts("equal to \"date -s\", but not need root privilege");
	}
	else if(argc == 2)
	{
		char *p = malloc(8+strlen(*argv));
		if(p==NULL) return 1;
		strcpy(p,"date -s ");
		strcat(p,argv[1]);
		uid_t euid = geteuid();
		setuid(euid);
		system(p);
		free(p);
	}
	else if(argc == 3)
	{
		char *p = malloc(11+strlen(*argv)+strlen(argv[1]));
		if(p==NULL) return 1;
		strcpy(p,"date -s \"");
		strcat(p,argv[1]);
		strcat(p," ");
		strcat(p,argv[2]);
		strcat(p,"\"");
		uid_t euid = geteuid();
		setuid(euid);
		system(p);
		free(p);
	}
	return 0;
}
```
参考
[linux：SUID、SGID详解](http://www.cnblogs.com/fhefh/archive/2011/09/20/2182155.html)
UNIX环境高级编程 8.11节
Centos man文档