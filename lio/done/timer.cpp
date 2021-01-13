#include "timer.h"
#include <map>

namespace GNET
{
namespace {
	class TimerKeeper : public Timer::Observer
	{
	private:
		typedef std::multimap<int, Thread::Runnable*> TimerTaskQueue;
		TimerTaskQueue tasks;
		Timer timer;
		TimerKeeper() { Timer::Attach(this); }
	public:
		static TimerKeeper &GetInstance() { static TimerKeeper s_instance; return s_instance; }
		void AddTask(Thread::Runnable *pTask, int waitsecs)
		{
			tasks.insert(std::make_pair(timer.Elapse() + waitsecs, pTask));
		}
		virtual void Update()
		{
			int nElapse = timer.Elapse();
			TimerTaskQueue::iterator it = tasks.begin();
			while(it != tasks.end())
			{
				if(it->first > nElapse) break;
				Thread::AddTask((*it).second, true);
				tasks.erase(it);
				it = tasks.begin();
			}
		}
	};
}

	time_t Timer::now = time(NULL);
	void Timer::AddTimerTask(Thread::Runnable *pTask, int waitsecs)
	{
		TimerKeeper::GetInstance().AddTask(pTask, waitsecs);
	}
}

