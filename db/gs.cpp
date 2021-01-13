#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
using namespace std;
#include "thread.h"
using namespace lcore;

bool stoped = false;
static void sigusr1_handler(int signum)
{
	puts("shutdown db by SIGUSR1");
	stoped = true;
	ThreadPool::Stop();
}

static bool add_signal()
{
	struct sigaction act;
	act.sa_handler = sigusr1_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	return sigaction(SIGUSR1, &act, NULL) == 0;
}

class mytask : public Runnable
{
public:
	void Run()
	{
		ThreadPool::AddTask(this);
	}
};

int main(int argc, char **argv)
{
	add_signal();
	ThreadPool::AddTask(new mytask());
	ThreadPool::Start();
	while(!stoped) sleep(1);
	return 0;
}
