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

