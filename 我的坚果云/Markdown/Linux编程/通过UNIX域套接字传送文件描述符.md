UNIX域套接字用于在同一台机器上运行的进程之间的通信。UNIX域套接字提供流和数据包两种接口。UNIX域套接字是套接字和管道之间的混合物。为了创建一对非命名的、相互连接的UNIX域套接字，可以使用socketpair函数。

	#include <sys/socket.h>
	int socketpair(int domain, int type, int protocol, int sockfd[2]);
pipe创建的管道，第一描述符的写端和第二描述符的读端都被关闭，socketpair创建的则是全双工UNIX域套接字。

使用面向网络的域套接字接口，可以创建命名UNIX域套接字，使无关进程之间也可以用UNIX域套接字进行通信。UNIX域套接字的地址由sockaddr_un结构表示：

	#include <sys/un.h>
	struct sockaddr_un {
		sa_family_t	sun_family; //AF_UNIX
		char		sun_path[108]; //pathname
	}
UNIX域套接字没有端口号，依靠唯一的路径名来标识。

UNIX域套接字可以用来传送文件描述符。描述符传递不是简单的传送一个int类型的描述符的值，而是在接收进程中创建一个新的描述符，这个描述符与发送进程的描述符指向内核文件表中的相同项。
当发送进程将描述符传送给接收进程后，通常它关闭该描述符。被发送者关闭的描述符并不真正关闭文件或设备，因为描述符在接收进程里仍视为打开的，即使接收者还没有明确地收到这个描述符。

传送文件描述符典型的应用场景就是进程池模式的网络服务程序：
1.控制进程负责监视接收网络连接请求，并将对应描述字通过描述符传递功能通知工作进程。
2.工作进程从网络连接读取和处理，并发回处理结果。
《APUE》提到的一个应用场景是：服务器可以是设置用户id程序，于是使其具有客户进程没有的附加权限。
## 流式UNIX域套接字传送文件描述符
下面是流式UNXI域套接字传送文件描述符的例子：
struct msghdr这个东西比较坑，sendmsg的时候就是把每个数据依次发送而已，recvmsg必须定义好msghdr每个字段的长度才能正确接收数据。
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
using namespace std;

int serv_listen(const char *name);
int serv_accept(int listenfd, uid_t *uidptr);
int cli_conn(const char *name);

int send_fd(int fd, int fd_to_send, const char *strmsg);
int recv_fd(int fd);

int main(int argc, char *argv[])
{
	//server without parameter
	if(argc == 1)
	{
		int lfd = serv_listen("un.domain");
		cout << "listen fd is " << lfd << endl;
		if(lfd < 0) return 0;
		uid_t uid;
		int fd = serv_accept(lfd, &uid);
		cout << "accept fd is " << fd << endl;
		if(fd < 0) return 0;
		cout << "client uid is " << uid << endl;
		while(true)
		{
			char buf[128];
			ssize_t n = read(fd, buf, sizeof(buf)-1);
			if(n <= 0) { close(fd); break; }
			buf[n] = 0;
			puts(buf);

			int open_fd = open("test.txt", O_RDONLY);
			cout << "open_fd is " << open_fd << endl;
			if(open_fd < 0) break;
			cout << "send_fd " << send_fd(fd, open_fd, "abcd") << " chars\n";
			close(open_fd);
			sleep(600);
		}
	}
	//client with some parameters
	else
	{
		int fd = cli_conn("un.domain");
		cout << "connect fd is " << fd << endl;
		if(fd < 0) return 0;
		for(int counter = 1; ;counter++)
		{
			char buf[16];
			sprintf(buf, "\n>>>%04d", counter);
			//对端关闭了socket再write会产生SIGPIPE,默认退出程序
			write(fd, buf, strlen(buf));
			puts("\n>>>");
			int trans_fd = recv_fd(fd);
			cout << "trans_fd is " << trans_fd << endl;
			if(trans_fd >= 0)
			{
				char bufs[64];
				int nbufs = read(trans_fd, bufs, 16);
				if(nbufs > 0)
				{
					bufs[nbufs] = 0;
					cout << "read data " << bufs << endl;
				}
			}
			sleep(2);
			//shutdown(fd, SHUT_WR);
			//close(fd);
		}
	}
	return 0;
}

int serv_listen(const char *name)
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0) return -1;
	unlink(name);
	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	int len = offsetof(sockaddr_un, sun_path) + strlen(un.sun_path);
	int rval, err;
	if(bind(fd, (sockaddr*)&un, len) < 0)
	{
		rval = -2;
		goto errout;
	}
	if(chmod(un.sun_path, S_IRWXU|S_IRWXO) < 0)
	{
		rval = -3;
		goto errout;
	}
	if(listen(fd, 10) < 0)
	{
		rval = -4;
		goto errout;
	}
	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int serv_accept(int listenfd, uid_t *uidptr)
{
	struct sockaddr_un un;
	socklen_t len = sizeof(un);
	int clifd = accept(listenfd, (struct sockaddr *)&un, &len);
	if(clifd < 0) return -1;

	len -= offsetof(struct sockaddr_un, sun_path);
	un.sun_path[len] = 0;

	cout << "client path is " << un.sun_path << endl;
	struct stat buf;
	int rval, err;
	if(stat(un.sun_path, &buf) < 0)
	{
		rval = -2;
		goto errout;
	}
	if(uidptr) *uidptr = buf.st_uid;
	unlink(un.sun_path);
	return clifd;

errout:
	err = errno;
	close(clifd);
	errno = err;
	return rval;
}

int cli_conn(const char *name)
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0) return -1;
	int err, rval;
	socklen_t len2;

	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%05d", getpid());
	int len = offsetof(sockaddr_un, sun_path) + strlen(un.sun_path);
	unlink(un.sun_path);
	//和tcp一样, 不bind直接connect也是可以的
	if(bind(fd, (sockaddr*)&un, len) < 0)
	{
		rval = -2;
		goto errout;
	}

	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len2 = offsetof(sockaddr_un, sun_path) + strlen(un.sun_path);
	if(connect(fd, (sockaddr*)&un, len2) < 0)
	{
		rval = -3;
		goto errout;
	}
	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int send_fd(int fd, int fd_to_send, const char *strmsg)
{
	//strmsg + \0 + char
	struct iovec vec[2];
	char tmp[2];
	tmp[0] = 0;
	if(fd_to_send >= 0) tmp[1] = 1;
	else tmp[1] = 0;
	vec[0].iov_base = (void *)strmsg;
	vec[0].iov_len = std::min(strlen(strmsg), size_t(255));
	vec[1].iov_base = tmp;
	vec[1].iov_len = 2;
	struct msghdr msg;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = vec;
	msg.msg_iovlen = 2;
	msg.msg_controllen = 0;
	//这里一定要从堆里分配空间
	//不能从栈上分配, 不然CMSG_DATA会把上个变量冲掉
	struct cmsghdr *cmp = (struct cmsghdr *)malloc(CMSG_LEN(sizeof(int)));
	//就不手动free了
	if(cmp && fd_to_send >= 0)
	{
		cmp->cmsg_level = SOL_SOCKET;
		cmp->cmsg_type = SCM_RIGHTS;
		cmp->cmsg_len = CMSG_LEN(sizeof(int));
		*(int *)CMSG_DATA(cmp) = fd_to_send;
		msg.msg_control = cmp;
		msg.msg_controllen = cmp->cmsg_len;
	}
	return sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
	char strmsg[256];
	memset(strmsg, 0, 256);
	struct iovec vec[1];
	vec[0].iov_base = strmsg;
	vec[0].iov_len = 255;
	struct msghdr msg;
	msg.msg_namelen = 0;
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	struct cmsghdr *cmp = (struct cmsghdr *)malloc(CMSG_LEN(sizeof(int)));
	if(cmp == NULL)
	{
		cout << "malloc error\n";
		return -1;
	}
	msg.msg_control = cmp;
	msg.msg_controllen = CMSG_LEN(sizeof(int));

	int nr = recvmsg(fd, &msg, 0);
	if(nr < 0)
	{
		cout << "recvmsg error " << errno << endl;
		return -2;
	}
	else if(nr == 0)
	{
		puts("connection closed by server");
		return -3;
	}
	cout << "recv " << nr << " chars\n";
	cout << "strmsg is " << strmsg << endl;
	int len = strlen(strmsg);
	if(strmsg[len] == 0 && strmsg[len+1])
		return *(int *)CMSG_DATA(cmp);
	else
		return -4;
}
````
## 数据包式UNIX域套接字传送文件描述符
为了搞清楚msghdr中的数据是如何序列化的，控制信息是如何发送的，尝试去抓包，搜到有人说socat可以，于是把上面的例子改成数据包方式发送：
```cpp
int bind_un(const char *name);
int send_fd(int fd, const char *dest, int fd_to_send, const char *strmsg);
int recv_fd(int fd);

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
		cout << "client fd is " << fd << endl;
		if(fd < 0) return 0;
		int open_fd = open("test.txt", O_RDONLY);
		cout << "open_fd is " << open_fd << endl;
		cout << "send_fd " << send_fd(fd, "un.domain", open_fd, "abcd") << " chars\n";
		close(open_fd);
		sleep(600);
	}
	else
	{
		int fd = bind_un("un.domain");
		cout << "bind fd is " << fd << endl;
		if(fd < 0) return 0;
		for(int counter = 1; ;counter++)
		{
			int trans_fd = recv_fd(fd);
			cout << "trans_fd is " << trans_fd << endl;
			if(trans_fd >= 0)
			{
				char bufs[64];
				int nbufs = read(trans_fd, bufs, 16);
				if(nbufs > 0)
				{
					bufs[nbufs] = 0;
					cout << "read data " << bufs << endl;
				}
			}
			sleep(1);
		}
	}
	return 0;
}

int bind_un(const char *name)
{
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(fd < 0) return -1;
	unlink(name);
	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	int len = offsetof(sockaddr_un, sun_path) + strlen(un.sun_path);
	int rval, err;
	if(bind(fd, (sockaddr*)&un, len) < 0)
	{
		rval = -2;
		goto errout;
	}
	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int send_fd(int fd, const char *dest, int fd_to_send, const char *strmsg)
{
	struct msghdr msg;
	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, dest);
	msg.msg_name = (sockaddr *)&un;
	msg.msg_namelen = sizeof(un);
	struct iovec vec[2];
	char tmp[2];
	tmp[0] = 0;
	if(fd_to_send >= 0) tmp[1] = 1;
	else tmp[1] = 0;
	vec[0].iov_base = (void *)strmsg;
	vec[0].iov_len = strlen(strmsg);
	vec[1].iov_base = tmp;
	vec[1].iov_len = 2;
	msg.msg_iov = vec;
	msg.msg_iovlen = 2;
	msg.msg_controllen = 0;
	struct cmsghdr *cmp = (struct cmsghdr *)malloc(CMSG_LEN(sizeof(int)));
	if(cmp && fd_to_send >= 0)
	{
		cmp->cmsg_level = SOL_SOCKET;
		cmp->cmsg_type = SCM_RIGHTS;
		cmp->cmsg_len = CMSG_LEN(sizeof(int));
		*(int *)CMSG_DATA(cmp) = fd_to_send;
		msg.msg_control = cmp;
		msg.msg_controllen = cmp->cmsg_len;
	}
	return sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
	struct msghdr msg;
	char strmsg[256];
	memset(strmsg, 0, 256);
	struct iovec vec[1];
	vec[0].iov_base = strmsg;
	vec[0].iov_len = 255;
	struct cmsghdr *cmp = (struct cmsghdr *)malloc(CMSG_LEN(sizeof(int)));
	if(cmp == NULL)
	{
		cout << "malloc error\n";
		return -1;
	}
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	msg.msg_control = cmp;
	msg.msg_controllen = CMSG_LEN(sizeof(int));
	msg.msg_flags = 0;

	int nr = recvmsg(fd, &msg, MSG_WAITALL);
	if(nr < 0)
	{
		cout << "recvmsg error " << errno << endl;
		return -2;
	}
	else if(nr == 0)
	{
		puts("connection closed by server");
		return -3;
	}
	puts(">>>");
	cout << "recv " << nr << " chars\n";
	cout << "strmsg is " << strmsg << endl;
	int len = strlen(strmsg);
	if(strmsg[len] == 0 && strmsg[len+1])
		return *(int *)CMSG_DATA(cmp);
	else
		return -4;
}
````
抓包结果：
`socat -L 100 -x -v unix-recvfrom:un.domain,mode=777,reuseaddr,fork unix-sendto:../un.domain`
![socat](https://img-blog.csdnimg.cn/20190302145540392.png)
数据流向是：client --> socat-recv --> socat-send --> server
然而socat-recv把控制信息都给丢掉了。。。
然后就没有再通过其他方式尝试了。
## 在UNIX域套接字上发送凭证
《APUE》17.4.2 后半部分讲了"如何在UNIX域套接字上发送凭证"，我实际测试了，发现sendmsg失败，查看了CMSG_NXTHDR的源码后发现在计算下一个cmsghdr的时候，会对齐地址的，所以实际占用空间会比两个cmsghdr大小之和大。
然而修正之后，sendmsg虽然成功了，但strace发现实际上并没有发送凭证，改为将文件描述符去掉，只发送凭证之后，虽然sendmsg端检测到发送了，但recvmsg仍然收不到。
不知为何。

参考：《APUE》17.3 17.4
