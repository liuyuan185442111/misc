#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>

//习题15.7
//当一个管道被写进程关闭后, 解释select如何处理该管道的输入描述符.
//read返回0
int main()
{
	int fd[2];
	pipe(fd);

	pid_t pid = fork();
	if(pid < 0) return -1;
	if(pid == 0)
	{
		//child
		close(fd[1]);
		fd_set fds;
		struct timeval tv;
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		for(;;)
		{
			FD_ZERO(&fds);
			FD_SET(fd[0], &fds);
			select(fd[0]+1, &fds, NULL, NULL, &tv);
			if(FD_ISSET(fd[0], &fds))
			{
				char buf[11];
				int n = read(fd[0], buf, 10);
				printf("recv %d\n",n);
				if(n < 0) exit(123);
				if(n == 0)
				{
					puts("closed by the opposite");
					exit(124);
				}
				buf[n] = 0;
				printf("recv %s\n", buf);
			}
		}
	}
	else
	{
		//parent
		close(fd[0]);
		write(fd[1], "abcdefghij", 10);
		write(fd[1], "abcdefghij", 10);
		sleep(2);
		close(fd[1]);
		waitpid(pid);
	}
	return 0;
}
