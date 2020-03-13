#include "thread.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#ifdef _REENTRANT_
#include "mutex.h"
#include <deque>
#endif

namespace GNET {
namespace Thread {

namespace {
#ifdef _REENTRANT_
typedef std::deque<Runnable*> TaskQueue;
TaskQueue s_tasks;
SpinLock s_spinlock; //for s_tasks
Runnable *FetchTask()
{
	Runnable *pTask = NULL;
	s_spinlock.Lock();
	if(!s_tasks.empty())
	{
		pTask = s_tasks.front();
		s_tasks.pop_front();
	}
	s_spinlock.UNLock();
	return pTask;
}
void *RunThread(void *index)
{
	pthread_detach(pthread_self());
	while(true)
	{
		if(s_tasks.empty())
		{
			while(s_tasks.empty())
				usleep(1000);
		}
		Runnable *pTask = FetchTask();
		if(!pTask) continue;
		try { pTask->Run(); }
		catch(...) { delete pTask; }
	}
	return index;
}
#else
typedef std::multimap<int, Runnable*> TaskQueue;
TaskQueue s_tasks;
static Runnable *FetchTask()
{
	Runnable *pTask = NULL;
	if(!s_tasks.empty())
	{
		TaskQueue::iterator it = s_tasks.begin();
		pTask = it->second;
		s_tasks.erase(it);
	}
	return pTask;
}
#endif

size_t s_pool_size = 1;

Policy *s_ppolicy = new Policy();
void sigusr1_handler(int signum)
{
	if(s_ppolicy && SIGUSR1 == signum)
	{
		s_ppolicy->SetQuit();
	}
}

void SetupDaemon()
{
	switch(fork())
	{
	case 0://child
		break;
	case -1://error
		exit(-1);
	default://parent
		exit(0);
	}
	setsid();
}
} //anonymous namespace


void SetPolicy(const Policy &policy)
{
	if(s_ppolicy != &policy)
	{
		delete s_ppolicy;
		s_ppolicy = policy.Clone();
	}
}

void AddTask(Runnable *task, bool bForced)
{
	if((task && !s_ppolicy) || s_ppolicy->CanAddTask(task, TaskSize(), bForced))
	{
#ifdef _REENTRANT_
		s_spinlock.Lock();
		s_tasks.push_back(task);
		s_spinlock.UNLock();
#else
		s_tasks.insert(std::make_pair(task->GetPriority(), task));
#endif
	}
	else if(task)
	{
		delete task;
	}
}

void Start(size_t pool_size, bool daemon)
{
	if(daemon) SetupDaemon();
	signal(SIGUSR1, sigusr1_handler);
#ifndef _REENTRANT_
	(void)pool_size;
	while(true)
	{
		Runnable *pTask = FetchTask();
		if(pTask)
		{
			try {
				pTask->Run();//task should "delete this;" in Run() when needed
			} catch( ... ) { delete pTask; }
		}
		if(s_ppolicy && s_ppolicy->GetQuit())
		{
			delete s_ppolicy;
			break;
		}
	}
	for(TaskQueue::iterator it = s_tasks.begin(); it != s_tasks.end(); ++it)
	{
		delete it->second;
	}
#else
	//one for IntervalTimer
	//one for PollIO
	if(pool_size < 3) s_pool_size = 3;
	else s_pool_size = pool_size;
	pthread_t pt;
	for(size_t i=s_pool_size; i>0; --i)
	{
		pthread_create(&pt, NULL, &RunThread, (void*)(&i));
	}
#endif
}

size_t TaskSize()
{
#ifdef _REENTRANT_
	SpinLock::Scoped l(s_spinlock);
#endif
	return s_tasks.size();
}
size_t ThreadSize() { return s_pool_size; }
}
}

