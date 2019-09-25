#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//12.9 线程和fork
//int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
//prepare由父进程在fork创建子进程之前调用, parent是在fork创建子进程以后,
//但在返回之间在父进程环境中调用的, child是在fork返回之前在子进程环境中调用的.

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void prepare()
{
	printf("preparing locks...\n");
	pthread_mutex_lock(&lock1);
	pthread_mutex_lock(&lock2);
}

void parent()
{
	printf("parent unlocking locks...\n");
	pthread_mutex_unlock(&lock1);
	pthread_mutex_unlock(&lock2);
}

void child()
{
	printf("child unlocking locks...\n");
	pthread_mutex_unlock(&lock1);
	pthread_mutex_unlock(&lock2);
}

void *thr_fn(void *arg)
{
	printf("thread started...\n");
	pause();
	return 0;
}

int main()
{
	int err;
	pid_t pid;
	pthread_t tid;
	pthread_atfork(prepare, parent, child);
	pthread_create(&tid, NULL, thr_fn, 0);
	sleep(2);
	printf("parent about to fork...\n");
	if((pid = fork()) == 0)
		printf("child returned from fork\n");
	else if(pid > 0)
		printf("parent returned from fork\n");
	exit(0);
}
