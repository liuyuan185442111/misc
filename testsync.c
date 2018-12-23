#include "sync.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void add_counter(pid_t pid, FILE *p)
{
	int i;
	fseek(p, 0, SEEK_SET);
	fscanf(p, "%d", &i);
	fseek(p, 0, SEEK_SET);
	fprintf(p, "%d", ++i);
	printf("pid=%d, counter=%d\n", pid, i);
	fflush(p);
}

int main()
{
	FILE *pf = fopen("ftemp", "w+");
	if(!pf) _exit(-1);
	fprintf(pf, "0");
	fflush(pf);
	int count = 900;

	if(!TELL_WAIT1()) _exit(-2);
	pid_t pid = fork();
	if(pid < 0) _exit(-3);
	if(pid == 0)
	{
		//child
		for(int i=0;i<count;++i)
		{
			WAIT();
			add_counter(getppid(), pf);
			TELL(getppid());
		}
		_exit(0);
	}

	//parent
	for(int i=0;i<count;++i)
	{
		TELL(pid);
		WAIT();
		add_counter(pid, pf);
	}

	fclose(pf);
	return 0;
}
