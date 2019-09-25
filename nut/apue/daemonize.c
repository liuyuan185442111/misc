#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

//13.3 守护进程的编程规则
void daemonize(const char *cmd)
{
	pid_t pid;
	struct rlimit rl;
	int i,fd0,fd1,fd2;
	struct sigaction sa;
	//become a session leader to lose controlling TTY
	if((pid=fork()) < 0) exit(1);
	else if(pid != 0) exit(0);
	setsid();
	//lose session leader, ensure future opens won't allocate controlling TTYs
	if((pid=fork()) < 0) exit(1);
	else if(pid != 0) exit(0);
	//ignore SIGHUP
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0) exit(0);
	//clear file creation mask
	umask(0);
	//not prevent file system from being unmounted
	if(chdir("/") < 0) exit(1);
	//close all open file descriptors
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0) exit(1);
	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for(i=0;i<rl.rlim_max;++i)
		close(i);
	//attach file descriptors 0, 1 and 2 to /dev/null
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	if(fd0 !=0 || fd1 != 1 || fd2 != 2) exit(1);
}

int main(int argc, char *argv[])
{
	char *cmd;
	FILE *fp;
	if((cmd = strrchr(argv[0],'/')) == NULL)
		cmd = argv[0];
	else
		++cmd;
	daemonize(cmd);
	fp = fopen("/tmp/getlog.out", "w");
	if(fp) fprintf(fp, "login name: %s\n", getlogin());
	fclose(fp);
	pause();
	return 0;
}
