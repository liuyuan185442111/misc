#include <signal.h>
#include <stddef.h>
#include "sync.h"

static volatile sig_atomic_t sigflag;
static sigset_t newmask, oldmask, zeromask;

static void sig_usr(int signo)
{
	sigflag = 1;
}

bool TELL_WAIT2()
{
	if(signal(SIGUSR1, sig_usr) != 0) return false;
	if(signal(SIGUSR2, sig_usr) != 0) return false;;
	if(sigemptyset(&zeromask) != 0) return false;
	if(sigemptyset(&newmask) != 0) return false;
	if(sigaddset(&newmask, SIGUSR1) != 0) return false;
	if(sigaddset(&newmask, SIGUSR2) != 0) return false;
	if(sigprocmask(SIG_BLOCK, &newmask, &oldmask) != 0) return false;
	return true;
}

void TELL_PARENT(pid_t pid)
{
	kill(pid, SIGUSR1);
}

void WAIT_PARENT()
{
	while (sigflag == 0)
		sigsuspend(&zeromask);
	sigflag = 0;
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void TELL_CHILD(pid_t pid)
{
	kill(pid, SIGUSR2);
}

void WAIT_CHILD()
{
	while (sigflag == 0)
		sigsuspend(&zeromask);
	sigflag = 0;
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}


bool TELL_WAIT1()
{
	if(signal(SIGUSR1, sig_usr) != 0) return false;
	if(sigemptyset(&zeromask) != 0) return false;
	if(sigemptyset(&newmask) != 0) return false;
	if(sigaddset(&newmask, SIGUSR1) != 0) return false;
	if(sigprocmask(SIG_BLOCK, &newmask, &oldmask) != 0) return false;
	return true;
}
void TELL(pid_t pid)
{
	kill(pid, SIGUSR1);
}
void WAIT()
{
	while (sigflag == 0) sigsuspend(&zeromask);
	sigflag = 0;
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
}
