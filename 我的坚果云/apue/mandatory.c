#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//14.3 记录锁
//记录锁锁定的是文件中的一个区域或整个文件

void set_fl(int fd, int flags) /* flags are file status flags to turn on */
{
	int        val;

	if ((val = fcntl(fd, F_GETFL, 0)) < 0)
	{
		printf("fcntl F_GETFL error");
		exit(1);
	}

	val |= flags;        /* turn on flags */

	if (fcntl(fd, F_SETFL, val) < 0)
	{
		printf("fcntl F_SETFL error");
		exit(1);
	}
}

//加锁和解锁一个文件区域
int lock_aux(int fd, int cmd, int type, off_t start, int whence, off_t len)
{
	struct flock lock;
	lock.l_type = type; // F_RDLCK, F_WRLCK, F_UNLCK
	lock.l_start = start; // byte offset, relative to l_whence
	lock.l_whence = whence; // SEEK_SET, SEEK_CUR, SEEK_END
	lock.l_len = len; // bytes, 0 means to EOF
	return fcntl(fd, cmd, &lock);
}

int main(int argc, char *argv[])
{
	int fd;
	pid_t pid;
	struct stat statbuf;
	char buf[5];
	if(argc != 2) {
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(1);
	}
	if((fd = open(argv[1],O_RDWR|O_CREAT|O_TRUNC, 0777)) < 0)
		exit(1);
	if(write(fd, "abcdef", 6) != 6)
		exit(1);
	//turn on set-group-ID and turn off group-execute
	if(fstat(fd, &statbuf) < 0)
		exit(1);
	if(fchmod(fd, (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0)
		exit(1);

	//TELL_WAIT();
	if((pid = fork()) < 0)
		exit(1);
	else if(pid > 0)
	{
		if(lock_aux(fd, F_SETLK, F_WRLCK, 0, SEEK_SET, 0) < 0)
			exit(1);
		//TELL_CHILD(pid);
		if(waitpid(pid,NULL,0) < 0)
			exit(1);
	}
	else
	{
		sleep(2);
		//WAIT_PARENT();
		set_fl(fd, O_NONBLOCK);
		if(lock_aux(fd, F_SETLK, F_RDLCK, 0, SEEK_SET, 0) != -1)
			exit(1);
		printf("read_lock of already-locked region returns %d\n", errno);

		if(lseek(fd,0,SEEK_SET) == -1)
			exit(1);
		if(read(fd,buf,2) < 0)
		{
			printf("read failed (mandatory locking works)\n");
			exit(1);
		}
		else
			printf("read OK (no mandatory locking), buf = %2.2s\n", buf);
	}
	return 0;
}
