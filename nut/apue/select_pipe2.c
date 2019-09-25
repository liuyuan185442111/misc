#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

//习题15.7
//当一个管道被读进程关闭后, 解释select如何处理该管道的输入描述符.
//对于已被读者关闭的引用管道的输出描述符来说, select表明该描述符是可写的.
//但当调用write时产生SIGPIPE信号, 如果忽略该信号或从信号处理程序返回时, write返回EPIPE错误.
void handler(int sig)
{
	puts("handle sig_pipe");
}

int main()
{
	int fd[2];
	pipe(fd);
	signal(SIGPIPE, handler);

	pid_t pid = fork();
	if(pid < 0) return -1;
	if(pid == 0)
	{
		//child
		close(fd[0]);
		fd_set fds;
		struct timeval tv;
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		for(;;)
		{
			FD_ZERO(&fds);
			FD_SET(fd[1], &fds);
			select(fd[1]+1, NULL, &fds, NULL, &tv);
			if(FD_ISSET(fd[1], &fds))
			{
				int n = write(fd[1], "abcdefghij", 10);
				printf("write %d\n", n);
				if(errno == EPIPE)
				{
					puts("closed by the opposite");
					exit(1234);
				}
			}
		}
	}
	else
	{
		//parent
		close(fd[1]);
		char buf[11];
		int n = read(fd[0], buf, 10);
		printf("recv %d\n",n);
		buf[n] = 0;
		printf("recv %s\n", buf);
		sleep(1);
		close(fd[1]);
		waitpid(pid);
	}
	return 0;
}
