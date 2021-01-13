#ifndef __POLLIO_H
#define __POLLIO_H

#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

#if defined USE_KEVENT
#include <sys/event.h>
#elif defined USE_EPOLL
#include <sys/epoll.h>
#elif defined USE_SELECT
#include <sys/select.h>
#endif

#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

#include "mutex.h"
#include "thread.h"
#include "timer.h"
#include "itimer.h"

namespace GNET
{

namespace {
	Thread::Mutex _locker_pollio_event("locker_pollio_event");
}

class PollController;
class PollIO
{
	friend class PollController;
	enum { POLLCLOSE = 0x80000000 };
	int  event;
	int  newevent;
	bool update;
#if defined USE_KEVENT
	int status;
#endif

	virtual void PollIn() = 0;
	virtual void PollOut() { }
	virtual void PollClose() { }

	void NotifyController();
	void FeedbackByController()
	{
		update   = false;
		event    = newevent;
		newevent = 0;
	}

protected:
	int fd;

	virtual ~PollIO()
	{
		while(close(fd) == -1 && errno == EINTR);
	}
#if defined USE_KEVENT
	PollIO(int x) : event(0), newevent(0), update(false), status(0), fd(x)
#else
	PollIO(int x) : event(0), newevent(0), update(false), fd(x)
#endif
	{
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
	}

public:
	void PermitRecv()
	{
		Thread::Mutex::Scoped l(_locker_pollio_event);
		if(update)
		{
			newevent |= POLLIN;
		} 
		else if(!(event & POLLIN))
		{
			update = true;
			newevent = event | POLLIN;
			NotifyController();
		}
	}
	void PermitSend()
	{
		Thread::Mutex::Scoped l(_locker_pollio_event);
		if(update)
		{
			newevent |= POLLOUT;
		} 
		else if(!(event & POLLOUT))
		{
			update = true;
			newevent = event | POLLOUT;
			NotifyController();
		}
	}
	void ForbidRecv()
	{
		Thread::Mutex::Scoped l(_locker_pollio_event);
		if(update)
		{
			newevent &= ~POLLIN;
		} 
		else if(event & POLLIN)
		{
			update = true;
			newevent = event & ~POLLIN;
			NotifyController();
		}
	}
	void ForbidSend()
	{
		Thread::Mutex::Scoped l(_locker_pollio_event);
		if(update)
		{
			newevent &= ~POLLOUT;
		} 
		else if(event & POLLOUT)
		{
			update = true;
			newevent = event & ~POLLOUT;
			NotifyController();
		}
	}
	void Close()
	{
		Thread::Mutex::Scoped l(_locker_pollio_event);
		newevent |= POLLCLOSE;
		if(!update) NotifyController();
	}

	class Task : public Thread::Runnable
	{
	public:
		Task(int prior=0) : Runnable(prior){}
		void Run();
	};
	static Task *GetPollIOTask() { static Task *ist = new Task(Thread::ThreadSize() == 1 ? INT_MAX : 0); return ist; }
};

class PollController
{
	friend class PollIO;
	typedef std::map<int, PollIO*> IOMap;
	typedef std::set<PollIO*> EventSet;
	typedef std::map<PollIO*, int> EventMap;

#if defined USE_KEVENT
	typedef std::vector<struct kevent> FDSet;
	static int kq;
	enum { READ_ENABLED = 1, WRITE_ENABLED = 2, READ_ADDED = 4, WRITE_ADDED = 8 };
#elif defined USE_EPOLL
	typedef std::vector<struct epoll_event> FDSet;
	static int ep;
#elif defined USE_SELECT
	typedef std::vector<int> FDSet;
	static fd_set rfds, wfds, all_rfds, all_wfds;
	static int maxfd;
#else
	struct PollFD : public pollfd
	{
		bool operator <  (const PollFD &rhs) const { return fd <  rhs.fd; }
		bool operator == (const PollFD &rhs) const { return fd == rhs.fd; }
	};
	typedef std::vector<PollFD> FDSet;
#endif

	static IOMap iomap;
	static IOMap ionew;
	static EventSet eventset;//有改动的IO的集合
	static FDSet fdset;//返回要处理的IO的集合

	static bool wakeup_flag;
	static Thread::Mutex locker_poll;
	static int Init();

#if defined USE_KEVENT
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int newevent = io->event;
		int fd       = io->fd;

		if(newevent & PollIO::POLLCLOSE)
		{
			iomap[fd] = NULL;
			delete io;
			return;
		}

		if(newevent & POLLOUT)
		{
			if(!(io->status & WRITE_ENABLED))
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_WRITE, (io->status & WRITE_ADDED) ? EV_ENABLE : EV_ADD, 0, 0, io);
				io->status |= (WRITE_ENABLED | WRITE_ADDED);
			}
		}
		else
		{
			if(io->status & WRITE_ENABLED)
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, io);
				io->status &= ~WRITE_ENABLED;
			}
		}

		if(newevent & POLLIN)
		{
			if(!(io->status & READ_ENABLED))
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_READ, (io->status & READ_ADDED) ? EV_ENABLE : EV_ADD, 0, 0, io);
				io->status |= (READ_ENABLED | READ_ADDED);
			}
		}
		else
		{
			if(io->status & READ_ENABLED)
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_READ, EV_DISABLE, 0, 0, io);
				io->status &= ~READ_ENABLED;
			}
		}
	}

	static void UpdateEvent()
	{
		fdset.clear();
		UpdateIOAndEvent();
	}

	static void TriggerEvent(const struct kevent &kv)
	{
		PollIO *io = (PollIO *)kv.udata;
		if(kv.filter == EVFILT_READ)  io->PollIn();
		if(kv.filter == EVFILT_WRITE) io->PollOut();
		io->PollClose();
	}
#elif defined USE_EPOLL
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int oldevent = event_pair.second;
		int newevent = io->event;
		int fd       = io->fd;

		struct epoll_event ev;
		if(newevent & PollIO::POLLCLOSE)
		{
			if(oldevent) epoll_ctl(ep, EPOLL_CTL_DEL, fd, &ev);
			iomap[fd] = NULL;
			delete io;
		}
		else
		{
			int status = 0;
			if(newevent & POLLOUT) status |= EPOLLOUT;
			if(newevent & POLLIN)  status |= EPOLLIN;
			if(status)
			{
				ev.events = status;
				ev.data.ptr = io;
				epoll_ctl(ep, (oldevent & (POLLOUT | POLLIN)) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, io->fd, &ev);
			}
			else if(oldevent)
			{
				epoll_ctl(ep, EPOLL_CTL_DEL, io->fd, &ev);
			}
		}
	}

	static void UpdateEvent()
	{
		UpdateIOAndEvent();
	}

	static void TriggerEvent(const struct epoll_event &ev)
	{
		PollIO *io = (PollIO *)ev.data.ptr;
		if(ev.events & (EPOLLIN | EPOLLERR | EPOLLHUP))
			io->PollIn();
		if(ev.events & EPOLLOUT)
			io->PollOut();
		io->PollClose();
	}
#elif defined USE_SELECT
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int newevent = io->event;
		int fd       = io->fd;

		if(newevent & PollIO::POLLCLOSE)
		{
			FD_CLR(fd, &all_rfds);
			FD_CLR(fd, &all_wfds);
			iomap[fd] = NULL;
			delete io;
		}
		else
		{
			if(newevent & POLLIN)  FD_SET(fd, &all_rfds); else FD_CLR(fd, &all_rfds);
			if(newevent & POLLOUT) FD_SET(fd, &all_wfds); else FD_CLR(fd, &all_wfds);
			maxfd = std::max(maxfd, fd);
		}
	}

	static void UpdateEvent()
	{
		UpdateIOAndEvent();
		rfds = all_rfds;
		wfds = all_wfds;
		fdset.clear();
		for(int i = 0; i <= maxfd; i++)
			if(FD_ISSET(i, &rfds) || FD_ISSET(i, &wfds))
				fdset.push_back(i);
	}

	static void TriggerEvent(int fd)
	{
		PollIO *io = iomap[fd];
		if(FD_ISSET(fd, &rfds)) io->PollIn();
		if(FD_ISSET(fd, &wfds)) io->PollOut();
		io->PollClose();
	}
#else
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int newevent = io->event;
		int fd       = io->fd;

		PollFD pfd;
		pfd.fd = fd;

		if(newevent & PollIO::POLLCLOSE)
		{
			iomap[fd] = NULL;
			delete io;
			fdset.erase(std::remove(fdset.begin(), fdset.end(), pfd), fdset.end());
		}
		else
		{
			FDSet::iterator it = std::lower_bound(fdset.begin(), fdset.end(), pfd);
			if(newevent)
			{
				if(it == fdset.end() || (*it).fd != fd)
					it = fdset.insert(it, pfd);
				(*it).events = newevent;
			}
			else
			{
				if(it != fdset.end() && (*it).fd == fd)
					fdset.erase(it);
			}
		}
	}

	static void UpdateEvent()
	{
		UpdateIOAndEvent();
	}

	static void TriggerEvent(const pollfd &fds)
	{
		PollIO *io = iomap[fds.fd];
		if(fds.revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL))
			io->PollIn();
		if(fds.revents & POLLOUT)
			io->PollOut();
		io->PollClose();
	}
#endif

	static void AddEvent(PollIO *io)
	{
		eventset.insert(io);
		WakeUp();
	}

	static void UpdateIOAndEvent()
	{
		EventMap map;
		{
			Thread::Mutex::Scoped l(_locker_pollio_event);
			//update iomap
			for(IOMap::const_iterator it = ionew.begin(), ie = ionew.end(); it != ie; ++it)
			{
				PollIO *io = it->second;
				iomap[it->first] = io;
				if(io->newevent) eventset.insert(io);
			}
			ionew.clear();
			//update eventset
			for(EventSet::iterator it = eventset.begin(), ie = eventset.end(); it != ie; ++it)
			{
				map.insert(std::make_pair(*it, (*it)->event));
				(*it)->FeedbackByController();
			}
			eventset.clear();
			wakeup_flag = true;
		}
		std::for_each(map.begin(), map.end(), std::ptr_fun(&LoadEvent));
	}

	static int Poll(int timeout)
	{
		Thread::Mutex::Scoped l(locker_poll);
		static int init_dummy = Init();
		UpdateEvent();
		ResumeTimer();
#if defined USE_KEVENT
		int nevents, nchanges = fdset.size();
		fdset.resize(iomap.size() * 2);
		if(timeout < 0)
		{
			nevents = kevent(kq, &fdset[0], nchanges, &fdset[0], fdset.size(), 0);
		}
		else
		{
			struct timespec ts;
			ts.tv_sec = timeout / 1000;
			ts.tv_nsec = (timeout - (ts.tv_sec * 1000)) * 1000000;
			nevents = kevent(kq, &fdset[0], nchanges, &fdset[0], fdset.size(), &ts);
		}
		wakeup_flag = false;
		SuspendTimer();
		if(nevents > 0)
			std::for_each(fdset.begin(), fdset.begin() + nevents, std::ptr_fun(&TriggerEvent));
#elif defined USE_EPOLL
		int maxevents = iomap.size();
		fdset.resize(maxevents);
		int nevents = epoll_wait(ep, &fdset[0], maxevents, timeout);
		wakeup_flag = false;
		SuspendTimer();
		if(nevents > 0)
			std::for_each(fdset.begin(), fdset.begin() + nevents, std::ptr_fun(&TriggerEvent));
#elif defined USE_SELECT
		int nevents;
		if(timeout < 0)
		{
			nevents = select(maxfd + 1, &rfds, &wfds, 0, 0);
		}
		else
		{
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - (tv.tv_sec * 1000)) * 1000;
			nevents = select(maxfd + 1, &rfds, &wfds, 0, &tv);
		}
		wakeup_flag = false;
		SuspendTimer();
		if(nevents > 0)
			std::for_each(fdset.begin(), fdset.end(), std::ptr_fun(&TriggerEvent));
		FD_ZERO(&rfds); FD_ZERO(&wfds);
#else
		int nevents = poll(&fdset[0], fdset.size(), timeout);
		wakeup_flag = false;
		SuspendTimer();
		if(nevents > 0)
			std::for_each(fdset.begin(), fdset.end(), std::ptr_fun(&TriggerEvent));
#endif
		return init_dummy;
	}

public:	
	static PollIO* Register(PollIO *io, bool init_permit_recv, bool init_permit_send)
	{
		Thread::Mutex::Scoped l(_locker_pollio_event);
		ionew[io->fd] = io;
		if(init_permit_recv) io->newevent |= POLLIN;
		if(init_permit_send) io->newevent |= POLLOUT;
		WakeUp();
		return io;
	}
	static void WakeUp();
};

#ifdef _REENTRANT_
class WakeupIO : public PollIO
{
	friend class PollController;
	static int writer;
	void PollIn() { for(char buff[64]; read(fd, buff, 64) == 64; ); }
	WakeupIO(int r, int w) : PollIO(r) { writer = w; fcntl(w, F_SETFL, fcntl(w, F_GETFL) | O_NONBLOCK); }
	~WakeupIO() { while(close(writer) == -1 && errno == EINTR); }
	static void WakeUp() { write(writer, "", 1); }
	static void Init()
	{
		int pds[2];
		pipe(pds);
		PollController::Register(new WakeupIO(pds[0], pds[1]), true, false);
	}
};
#else
class WakeupIO
{
	friend class PollController;
	static void WakeUp() { }
	static void Init()   { }
};
#endif

inline void PollIO::NotifyController()
{
	PollController::AddEvent(this);
}

inline int PollController::Init()
{
	signal(SIGPIPE, SIG_IGN);
#if defined USE_KEVENT
	kq = kqueue();
#elif defined USE_EPOLL
	ep = epoll_create(8192);
#elif defined USE_SELECT
	maxfd = 0; FD_ZERO(&rfds); FD_ZERO(&wfds);
#endif
	WakeupIO::Init();
	return 0;
}

inline void PollController::WakeUp()
{
	if(wakeup_flag)
	{
		wakeup_flag = false;
		WakeupIO::WakeUp();
	}
}

}//namespace GNET

#endif
