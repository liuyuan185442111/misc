#include <string>
#include <list>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <assert.h>
using namespace std;
#include "un.h"

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		int lfd = serv_listen("test.domain");
		cout << "listen fd is " << lfd << endl;
		uid_t uid;
		int fd = serv_accept(lfd, &uid);
		cout << "accept fd is " << fd << endl;
		cout << "client uid is " << uid << endl;
		while(true)
		{
			char buf[128];
			ssize_t n = read(fd, buf, sizeof(buf)-1);
			if(n <= 0) { close(fd); break; }
			buf[n] = 0;
			puts(buf);
			//write(fd, buf, strlen(buf));
			cout << "send_fd " << send_fd(fd, 0, NULL) << endl;
		}
	}
	else
	{
		int fd = cli_conn("test.domain");
		cout << "connect fd is " << fd << endl;
		char buf[8];
		for(int counter = 1; ;counter++)
		{
			//对端关闭了socket write会产生SIGPIPE 默认是退出程序
			sprintf(buf, ">>>%4d", counter);
			write(fd, buf, 7);
			sleep(2);
			recv_fd(fd);
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
	if(listen(fd, 10) < 0)
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
	struct sockaddr_un un;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%05d", getpid());
	int len = offsetof(sockaddr_un, sun_path) + strlen(un.sun_path);
	unlink(un.sun_path);
	int err, rval;
	socklen_t len2;
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

int send_fd(int fd, int fd_to_send, const char *errmsg)
{
	char buf[2];
	struct iovec vec[1];
	vec[0].iov_base = buf;
	vec[0].iov_len = 1;
	struct msghdr msg;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	return sendmsg(fd, &msg, 0);
}

int recv_fd(int fd)
{
	char buf[2];
	struct iovec vec[1];
	vec[0].iov_base = buf;
	vec[0].iov_len = 1;
	struct msghdr msg;
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	int nr = recvmsg(fd, &msg, 0);
	if(nr < 0)
	{
		puts("recvmsg error");
		cout << errno << endl;
		return -1;
	}
	else if(nr == 0)
	{
		puts("connection closed by server");
		return -2;
	}
	puts("succeed");
	return 0;
}

