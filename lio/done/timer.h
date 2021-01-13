#ifndef __TIMER_H
#define __TIMER_H

#include <vector>
#include <functional>
#include <algorithm>
#include <time.h>
#include "thread.h"

namespace GNET
{

class Timer
{
public:
	struct Observer
	{
		virtual ~Observer() { }
		virtual void Update() = 0;
	};
private:
	static std::vector<Observer*>& observers() { static std::vector<Observer*> obs; return obs; }
	static time_t now;
	time_t start;
public:
	Timer() : start(now) { }
	int Elapse() const { return now - start; }
	void Reset() { start = now; }
public:
	static void Attach(Observer *o) { observers().push_back(o); }
	static void Detach(Observer *o) { observers().erase(std::remove(observers().begin(), observers().end(), o), observers().end()); }
	//called by PollController::Task
	static void Update()
	{
		time_t cur_time = time(NULL);
		if(cur_time > now)
		{
			now = cur_time;
			std::for_each(observers().begin(), observers().end(), std::mem_fun(&Observer::Update));
		}
	}
	static time_t GetTime() { return now; }
	static void AddTimerTask(Thread::Runnable *pTask, int waitsecs);
};

}
#endif
