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

int send_fd(int fd, int fd_to_send, const char *errmsg);
int recv_fd(int fd, uid_t *uidptr);

int main(int argc, char *argv[])
{
	//server
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
		int open_fd = open("fdun.txt", O_RDONLY);
		cout << "open_fd is " << open_fd << endl;
		while(true)
		{
			char buf[128];
			ssize_t n = read(fd, buf, sizeof(buf)-1);
			if(n <= 0) { close(fd); break; }
			buf[n] = 0;
			puts(buf);
			int n_send_fd = send_fd(fd, open_fd, "abcd");
			cout << "send_fd " << n_send_fd << " chars\n";
			if(n_send_fd < 0)
				cout << "errno is " << errno << endl;
			//被发送者关闭的描述符并不真正关闭文件或设备,
			//因为描述符在接收进程里仍视为打开的,
			//即使接收者还没有明确地收到这个描述符
			close(open_fd);
			sleep(600);
		}
	}
	//client
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
			uid_t uid = 0;
			int trans_fd = recv_fd(fd, &uid);
			cout << "trans_fd is " << trans_fd << endl;
			if(trans_fd >= 0)
			{
				cout << "uid is " << uid << endl;
				char bufs[256];
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
	//errmsg + \0 + char
	struct iovec vec[2];
	char tmp[2];
	tmp[0] = 0;
	if(fd_to_send >= 0) tmp[1] = 1;
	else tmp[1] = 0;
	vec[0].iov_base = (void *)errmsg;
	vec[0].iov_len = strlen(errmsg);
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
	size_t cmpp_size = CMSG_ALIGN(CMSG_LEN(sizeof(int))) + CMSG_ALIGN(CMSG_LEN(sizeof(struct ucred)));
	struct cmsghdr *cmpp = (struct cmsghdr *)malloc(cmpp_size);
	struct cmsghdr *cmp = cmpp;
	cout << sizeof(ucred) << endl;
	if(cmpp && fd_to_send >= 0)
	{
		msg.msg_control = cmpp;
		msg.msg_controllen = CMSG_LEN(sizeof(struct ucred));//cmpp_size;
		/*
		cmp->cmsg_level = SOL_SOCKET;
		cmp->cmsg_type = SCM_RIGHTS;
		cmp->cmsg_len = CMSG_LEN(sizeof(int));
		cout << cmp << endl;
		*(int *)CMSG_DATA(cmp) = fd_to_send;
		cmp = CMSG_NXTHDR(&msg, cmp);
		cout << cmp << endl;
		*/
		cmp->cmsg_level = SOL_SOCKET;
		cmp->cmsg_type = SCM_CREDENTIALS;
		cmp->cmsg_len = CMSG_LEN(sizeof(struct ucred));
		struct ucred *ucredp = (struct ucred *)CMSG_DATA(cmp);
		ucredp->uid = geteuid();
		ucredp->gid = getegid();
		ucredp->pid = getpid();
	}
	int ret = sendmsg(fd, &msg, 0);
	if(cmpp) free(cmpp);
	return ret;
}

int recv_fd(int fd, uid_t *uidptr)
{
	char errmsg[256];
	memset(errmsg, 0, 256);
	struct iovec vec[1];
	vec[0].iov_base = errmsg;
	vec[0].iov_len = 255;
	struct msghdr msg;
	msg.msg_namelen = 0;
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	size_t cmpp_size = CMSG_ALIGN(CMSG_LEN(sizeof(int))) + CMSG_ALIGN(CMSG_LEN(sizeof(struct ucred)));
	struct cmsghdr *cmp = (struct cmsghdr *)malloc(cmpp_size);
	//不free了
	if(cmp == NULL)
	{
		cout << "malloc error\n";
		return -9;
	}
	msg.msg_control = cmp;
	msg.msg_controllen = cmpp_size;

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
	cout << "recv " << nr << " chars\n";
	cout << "errmsg is " << errmsg << endl;
	int len = strlen(errmsg);
	if(errmsg[len] == 0 && errmsg[len+1])
	{
		int newfd;
		for(cmp=CMSG_FIRSTHDR(&msg); cmp != NULL; cmp = CMSG_NXTHDR(&msg,cmp))
		{
			if(cmp->cmsg_level != SOL_SOCKET)
				continue;
			switch(cmp->cmsg_type)
			{
				case SCM_RIGHTS:
					cout << "----------------\n";
					newfd = *(int *)CMSG_DATA(cmp);
					break;
				case SCM_CREDENTIALS:
					cout << "================\n";
					if(uidptr){
						*uidptr = ((struct ucred *)CMSG_DATA(cmp))->uid;
					cout << *uidptr << endl;}
					break;
			}
		}
		return newfd;
	}
	else
	{
		return -3;
	}
}

