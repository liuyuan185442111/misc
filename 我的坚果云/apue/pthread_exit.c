#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

//11.5 线程终止
struct foo { int a,b,c,d;};

struct foo foo;

void printfoo(const char *s, const struct foo *fp)
{
	printf(s);
	printf("   structure at 0x%x\n", (unsigned long)fp);
	printf("   a=%d\n", fp->a);
	printf("   b=%d\n", fp->b);
	printf("   c=%d\n", fp->c);
	printf("   d=%d\n", fp->d);
}

void *thr_fn1(void *arg)
{
	foo.a = 1;
	foo.b = 2;
	foo.c = 3;
	foo.d = 4;
	printfoo("thread 1:\n", &foo);
	pthread_exit((void*)&foo);
}

void *thr_fn2(void *arg)
{
	printf("thread 2: Id is %d\n", pthread_self());
	pthread_exit((void*)0);
}

//void pthread_exit(void *rval_ptr);
//int pthread_join(pthread_t thread, void **rval_ptr);
//进程中的其他线程可以通过调用pthread_join函数访问到某线程调用pthread_exit函数的参数
int main()
{
	int err;
	pthread_t tid1,tid2;
	struct foo *fp;
	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
	if(err) _exit(-1);
	err = pthread_join(tid1, (void*)&fp);
	if(err) _exit(-1);
	sleep(1);
	printf("parent staring second thread\n");
	err = pthread_create(&tid2, NULL, thr_fn2, NULL);
	if(err) _exit(-1);
	sleep(1);
	printfoo("parent:\n", fp);
	return 0;
}
