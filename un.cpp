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
#include "errno.h"

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		int lfd = serv_listen("test.domain");
		cout << "listen fd is " << lfd << endl;
		if(lfd < 0) return 0;
		uid_t uid;
		int fd = serv_accept(lfd, &uid);
		cout << "accept fd is " << fd << endl;
		cout << "client uid is " << uid << endl;
		if(fd < 0) return 0;
		while(true)
		{
			char buf[128];
			ssize_t n = read(fd, buf, sizeof(buf)-1);
			if(n <= 0) { close(fd); break; }
			buf[n] = 0;
			puts(buf);
			//write(fd, buf, strlen(buf));
			for(;;){
			write(fd, ":666:", 5);
			cout << "send_fd " << send_fd(fd, 9356, "987654") << " chars\n";
			write(fd, ":999:", 5);
			//cout << "errno " << errno << endl;
			}
		}
	}
	else
	{
		int fd = cli_conn("test.domain");
		cout << "connect fd is " << fd << endl;
		if(fd < 0) return 0;
		for(int counter = 1; ;counter++)
		{
			char buf[8];
			sprintf(buf, ">>>%4d", counter);
			//对端关闭了socket write会产生SIGPIPE 默认是退出程序
			write(fd, buf, 7);
			recv_fd(fd);
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
	struct iovec vec[2];
	vec[0].iov_base = (void *)errmsg;
	vec[0].iov_len = strlen(errmsg);
	vec[1].iov_base = &fd_to_send;
	vec[1].iov_len = sizeof(fd_to_send);
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
	char errmsg[256];
	memset(errmsg, 0, 256);
	int trans_fd;
	struct iovec vec[2];
	vec[0].iov_base = errmsg;
	vec[0].iov_len = 255;
	vec[1].iov_base = &trans_fd;
	vec[1].iov_len = sizeof(trans_fd);
	struct msghdr msg;
	msg.msg_namelen = 0;
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	msg.msg_controllen = 0;
	int nr = recvmsg(fd, &msg, 0);
	if(nr < 0)
	{
		cout << "recvmsg error " << errno << endl;
		return -1;
	}
	else if(nr == 0)
	{
		puts("connection closed by server");
		return -2;
	}
	puts(">>>");
	cout << errmsg << endl;
	cout << trans_fd << endl;
	return 0;
}

