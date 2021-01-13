#include "itimer.h"
#include "mutex.h"
#include <signal.h>
#include <sys/time.h>
#include <map>
#include <stdint.h>
#ifdef _REENTRANT_
#include <pthread.h>
#endif

namespace GNET{
namespace IntervalTimer
{
namespace {
	class TimerTask : public Observer
	{
		Thread::Runnable *runnable;
	public: 
		TimerTask(Thread::Runnable *r) : runnable(r){}
		bool Update()
		{
			Thread::AddTask(runnable);
			delete this;
			return false;
		}
	};
	struct IntervalObserver
	{
		int interval;
		Observer *observer;
		IntervalObserver(int i, Observer *o) : interval(i),observer(o){}
	};

	typedef std::multimap<int64_t, IntervalObserver> Observers;
	Observers observers;
	Thread::Mutex locker;//for observers

	bool stop = true;
	bool triggered = false;
	int interval = DEFAULT_INTERVAL;
	struct timeval base;
	struct timeval now;
	int64_t tick_now;
	sigset_t mask;

	void Handler(int signum)
	{
		if(signum == SIGALRM)
			triggered = true;
	}

	void AttachObserver(int interval, Observer *observer)
	{
		observers.insert(Observers::value_type(tick_now + interval, IntervalObserver(interval, observer)));
	}
	void AttachObserver(const IntervalObserver &io)
	{
		observers.insert(Observers::value_type(tick_now + io.interval, io));
	}
	void Update()
	{
		Observers::iterator it;
		{
			Thread::Mutex::Scoped lock(locker);
			gettimeofday(&now, NULL);
			tick_now = ((int64_t)(now.tv_sec - base.tv_sec)*1000000 + now.tv_usec - base.tv_usec)/interval;
			it = observers.begin();
		}
		while(it != observers.end()) {
			if(it->first > tick_now) break;
			bool add = it->second.observer->Update();
			{
				Thread::Mutex::Scoped lock(locker);
				if(add && !stop) AttachObserver(it->second);
				observers.erase(it);
				it = observers.begin();
			}
		}
	}

	void SetITimer()
	{
		struct itimerval value;
		value.it_interval.tv_sec = interval/1000000;
		value.it_interval.tv_usec = interval%1000000;
		value.it_value.tv_sec = interval/1000000;
		value.it_value.tv_usec = interval%1000000;
		setitimer(ITIMER_REAL, &value, NULL);
	}

#ifdef _REENTRANT_
	class TimerLancher : public Thread::Runnable
	{
		virtual void Run()
		{
			sigset_t sigs;
			sigfillset(&sigs);
			pthread_sigmask(SIG_BLOCK, &sigs, NULL);
			SetITimer();

			//sigsuspend函数会替换当前线程的屏蔽字, 然后进入pause状态,
			//等到期望的信号到来, 执行完对应的信号处理函数后返回
			sigfillset(&sigs);
			sigdelset(&sigs, SIGALRM);
			while(!stop)
			{
				sigsuspend(&sigs);
				Update();
			}
			setitimer(ITIMER_REAL, NULL, NULL);
		}
	};
#endif
}

	bool StartTimer(int usec)
	{
		if(!stop) return false;
		stop = false; 
		if(usec > 0) interval = usec;

		sigemptyset(&mask);
		sigaddset(&mask, SIGALRM);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		signal(SIGALRM, Handler);
#ifndef _REENTRANT_
		SetITimer();
#else
		Thread::AddTask(new TimerLancher(), true);
#endif
		gettimeofday(&base, NULL);
		now = base;
		tick_now = 0;
		return true;
	}
	bool Attach(Observer *observer, int interval)
	{
		if(stop) return false;
		if(interval <= 0) interval = 1;
		Thread::Mutex::Scoped lock(locker);
		AttachObserver(interval, observer);
		return true;
	}
	void StopTimer() { stop = true; }
	int Interval() { return interval; }
	void AddTimerTask(Thread::Runnable *task, int delay)
	{
		Attach(new TimerTask(task), delay);
	}

	//block SIGALRM
	void Suspend()
	{
		sigprocmask(SIG_BLOCK, &mask, NULL);
		if(triggered)
		{
			Update();
			triggered = false;
		}
	}
	//resume SIGALRM
	void Resume()
	{
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}
}//namespace IntervalTimer
}//namespace GNET

