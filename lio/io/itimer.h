#ifndef __ITIMER_H
#define __ITIMER_H

#include "thread.h"

//net IO calls ResumeTimer() before wait, calls SuspendTimer() after wait
#ifndef _REENTRANT_
#define ResumeTimer  IntervalTimer::Resume
#define SuspendTimer IntervalTimer::Suspend
#else
#define ResumeTimer() 
#define SuspendTimer() 
#endif

//microseconds per tick
#define DEFAULT_INTERVAL 100000

namespace GNET
{
namespace IntervalTimer
{
	struct Observer
	{
		virtual ~Observer() {}
		virtual bool Update() = 0;//return false means just doing once
	};

	bool StartTimer(int usec=0);//usec microseconds per tick
	bool Attach(Observer *o, int interval);//interval ticks
	void StopTimer();
	int Interval();
	void AddTimerTask(Thread::Runnable *task, int delay);//delay ticks

	//block SIGALRM
	void Suspend();
	//resume SIGALRM
	void Resume();
}
}

#endif
