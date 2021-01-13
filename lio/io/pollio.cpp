#include "pollio.h"
#include "stdio.h"

namespace GNET
{

PollController::IOMap PollController::iomap;
PollController::FDSet PollController::fdset;
PollController::IOMap PollController::ionew;
PollController::EventSet PollController::eventset;
Thread::Mutex PollController::locker_poll("PollController::locker_poll");
bool PollController::wakeup_flag  = false;

#if defined USE_KEVENT
int PollController::kq;
#elif defined USE_EPOLL
int PollController::ep;
#elif defined USE_SELECT
int PollController::maxfd;
fd_set PollController::rfds;
fd_set PollController::wfds;
fd_set PollController::all_rfds;
fd_set PollController::all_wfds;
#endif

#ifdef _REENTRANT_
int WakeupIO::writer;
#endif

void PollIO::Task::Run()
{
	if(Thread::ThreadSize() == 1)
	{
		PollController::Poll(1000);//1s
		Timer::Update();
	}
	else
	{
		PollController::Poll(-1);
	}
	Thread::AddTask(this, true);
}

}
