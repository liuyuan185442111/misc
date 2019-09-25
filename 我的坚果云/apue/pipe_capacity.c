#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
	int i,n;
	int val;
	int fd[2];
	pipe(fd);

	val = fcntl(fd[1], F_GETFL, 0);
	val |= O_NONBLOCK;
	fcntl(fd[1], F_SETFL, val);

	for(n=0;;++n)
	{
		if((i=write(fd[1],"a",1))!=1)
		{
			printf("write ret %d, ", i);
			break;
		}
	}
	printf("pipe capacity=%d\n",n);
	return 0;
}
