APUE 习题15.15，一个XSI共享存储的测试程序：
```c
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h>

static volatile sig_atomic_t sigflag;
static sigset_t newmask, oldmask, zeromask;

static void sig_usr(int signo)
{
	sigflag = 1;
}

bool TELL_WAIT()
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
    while(sigflag == 0) sigsuspend(&zeromask);
    sigflag = 0;
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}


static int update(int *ptr)
{
	return ((*ptr)++);
}

int main()
{
	int shmid = shmget(IPC_PRIVATE, sizeof(int), 0600);
	assert(shmid != -1);

	TELL_WAIT();

	pid_t pid = fork();
	if(pid < 0) return 3;
	else if(pid > 0)
	{
		void *area = shmat(shmid, 0, 0);
		assert(area != (void*)-1);
		printf("parent shm addr: %p\n", area);
		for(int i=0; i<100; i+=2)
		{
			printf("parent %d\n", update((int*)area));
			TELL(pid);
			WAIT();
		}
		shmdt(area);
	}
	else
	{
		void *area = shmat(shmid, 0, 0);
		assert(area != (void*)-1);
		printf("child shm addr: %p\n", area);
		for(int i=1; i<101; i+=2)
		{
			WAIT();
			printf("child %d\n", update((int*)area));
			TELL(getppid());
		}
		shmdt(area);
		shmctl(shmid, IPC_RMID, 0);
	}
	return 0;
}
```
gcc -std=c99 -D_XOPEN_SOURCE test.c
运行结果不对：
parent shm addr: 0x7f9a4476e000
parent 0
child shm addr: 0x7f9a4476e000
child 1
parent 2
然后查看子进程处于\<defunct>状态，子进程退出了，父进程还在WAIT。
修改父进程，调用wait()获取子进程状态是10，正好是信号SIGUSR1的值，怀疑是子进程收到SIGUSR1就退出了，查了SIGUSR1的默认动作是终止程序。在WAIT()中sigsuspend返回之后重新设置信号处理程序，输出变正常了。

结合man和帖子[signal()注册的信号处理函数里面，不需要再次注册信号处理函数吗? ](http://bbs.chinaunix.net/thread-4175893-1-1.html)：
The kernel's signal() system call provides System V semantics. System V会在处理信号之后将信号处理程序重置为SIG_DFL。如果编译时加了额外的选项例如-std=c99，那么用内核的signal()系统调用。默认情况下使用sigaction函数的方式。

尝试去掉-std=c99并调整代码并没有作用，可能是还带了-D_XOPEN_SOURCE；以下两种方式是可以的：
g++ test.c
gcc -std=gnu99  test.c

APUE 10.14节对sigaction函数明确指出：一旦对给定的信号设置了一个动作，那么在调用sigaction显式地改变它之前，该设置一直有效。所以将

    if(signal(SIGUSR1, sig_usr) != 0) return false;
替换为

    struct sigaction act;
    act.sa_handler = sig_usr;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction(SIGUSR1, &act, NULL) != 0) return false;
也可以解决问题。

结论：
最好使用sigaction函数替代signal函数。
