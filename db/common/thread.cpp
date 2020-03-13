#include "thread.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#ifdef _REENTRANT_
#include "mutex.h"
#include <deque>
#endif

namespace lcore {

namespace {
bool stoped = false;
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
	while(!stoped)
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

namespace ThreadPool {
void AddTask(Runnable *task)
{
	if(task)
	{
#ifdef _REENTRANT_
		s_spinlock.Lock();
		s_tasks.push_back(task);
		s_spinlock.UNLock();
#else
		s_tasks.insert(std::make_pair(task->GetPriority(), task));
#endif
	}
}

void Start(size_t pool_size, bool daemon)
{
	if(daemon) SetupDaemon();
#ifndef _REENTRANT_
	(void)pool_size;
	while(!stoped)
	{
		Runnable *pTask = FetchTask();
		if(pTask)
		{
			try {
				pTask->Run();//task should "delete this;" in Run() when needed
			} catch( ... ) { delete pTask; }
		}
	}
#else
	pthread_t pt;
	for(size_t i=pool_size; i>0; --i)
	{
		pthread_create(&pt, NULL, &RunThread, (void*)(&i));
	}
#endif
}

void Stop()
{
	stoped = true;
}

size_t TaskSize()
{
#ifdef _REENTRANT_
	SpinLock::Scoped l(s_spinlock);
#endif
	return s_tasks.size();
}
}

}

