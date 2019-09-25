#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

//gcc sig2str.c -std=gnu99
int sig2str(int signo, char *str)
{
	char *s = strsignal(signo);
	memcpy(str, s, strlen(s)+1);
	return 0;
}

int main()
{
	char buf[1024];
	for(int i=0;i<66;++i)
	{
		sig2str(i, buf);
		printf("signal %d:",i);
		puts(buf);
	}
	return 0;
}
