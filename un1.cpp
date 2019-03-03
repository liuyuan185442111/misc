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

