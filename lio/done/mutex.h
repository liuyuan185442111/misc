#ifndef __MUTEX_H
#define __MUTEX_H

#if defined _DEADLOCK_DETECT_
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>

namespace GNET
{
namespace Thread
{
	class Condition;
	class Mutex
	{
		friend class Condition;
		pthread_mutex_t mutex;
		std::string     identification;
		bool            isrecursive;
		Mutex(const Mutex& rhs);
		void Init(bool recursive)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE_NP : PTHREAD_MUTEX_ERRORCHECK_NP);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}
	public:
		~Mutex() 
		{
			while(pthread_mutex_destroy(&mutex) == EBUSY) { Lock(); UNLock(); }
		}
		explicit Mutex(bool recursive=false) : identification(""),isrecursive(false)
		{
			Init(recursive);
		}
		explicit Mutex(const char *id, bool recursive=false) : identification(id),isrecursive(recursive)
		{
			Init(recursive);
		}

		const std::string& Identification() const { return identification; }
		std::string Identification() { return identification; }

		void Lock()   { pthread_mutex_lock(&mutex);   }
		void UNLock() { pthread_mutex_unlock(&mutex); }
		bool TryLock() { return pthread_mutex_trylock(&mutex) == 0; }

		class Detector
		{
			typedef std::map<Mutex*, pthread_t> GRQ; //ownership graph:lock owned by tid
			typedef std::map<pthread_t, Mutex*> PRQ; //request graph:tid wait for lock
			GRQ grq;
			PRQ prq;
			pthread_mutex_t locker;
			void DeadLock()
			{
				FILE *fp = fopen("deadlock", "w+");
				fprintf(fp, "[request graph]\n");
				for(PRQ::iterator it = prq.begin(), ie = prq.end(); it != ie; ++it)
					fprintf(fp, "\tTID[%lx]:Mutex[%p](%s)\n", (*it).first,(*it).second,(*it).second->Identification().c_str());
				typedef std::map<pthread_t, std::vector<Mutex*> > SNAPSHOT;
				SNAPSHOT snapshot;
				for(GRQ::iterator it = grq.begin(), ie = grq.end(); it != ie; ++it)
				{
					snapshot[it->second].push_back(it->first);
				}
				fprintf(fp, "[ownership graph]\n");
				for(SNAPSHOT::iterator it = snapshot.begin(), ie = snapshot.end(); it != ie; ++it)
				{
					fprintf(fp, "\tTID[%lx]:", (*it).first);
					for(std::vector<Mutex*>::iterator i = (*it).second.begin(), e = (*it).second.end(); i != e; ++i)
						fprintf(fp, "Mutex[%p](%s),", *i,(*i)->Identification().c_str());
					fprintf(fp, "\n");
				}
				fclose(fp);
				abort();
			}
		public:
			Detector() 
			{
				pthread_mutexattr_t attr;
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
				pthread_mutex_init(&locker, &attr);
				pthread_mutexattr_destroy(&attr);
			}
			void Lock(Mutex *mutex)
			{
				pthread_t tid = pthread_self();
				pthread_t r = tid;
				pthread_mutex_lock(&locker);
				prq.insert(std::make_pair(tid, mutex));
				GRQ::iterator it_grq;
				PRQ::iterator it_prq;
				if(mutex->isrecursive)
				{
					it_grq = grq.find(mutex);
					if(it_grq != grq.end() && (*it_grq).second == r)
						goto recursive;
				}
				while((it_prq = prq.find(r)) != prq.end() && (it_grq = grq.find((*it_prq).second)) != grq.end())
					if((r = (*it_grq).second) == tid) DeadLock();
			recursive:
				pthread_mutex_unlock(&locker);
				mutex->Lock();
				pthread_mutex_lock(&locker);
				prq.erase(tid);
				grq.insert(std::make_pair(mutex, tid));
				pthread_mutex_unlock(&locker);
			}
			void UNLock(Mutex *mutex)
			{
				mutex->UNLock();
				pthread_mutex_lock(&locker);
				grq.erase(mutex);
				pthread_mutex_unlock(&locker);
			}
			static Detector& GetInstance() { static Detector tmp; return tmp; }
		};

		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped() { Detector::GetInstance().UNLock(mx); }
			explicit Scoped(Mutex& m) : mx(&m) { Detector::GetInstance().Lock(mx); }
		};

		typedef Scoped RDScoped;
		typedef Scoped WRScoped;
	};

	typedef Mutex SpinLock;
	typedef Mutex RWLock;

	class Condition
	{
		pthread_cond_t cond;
		Condition(const Condition& rhs) { }
	public:
		~Condition() 
		{
			while(pthread_cond_destroy(&cond) == EBUSY) { NotifyAll(); }
		}
		explicit Condition() { pthread_cond_init( &cond, NULL ); }
		int Wait(Mutex & mutex) { return pthread_cond_wait(&cond, &mutex.mutex); }
		int TimedWait(Mutex & mutex, int nseconds)
		{
			if(nseconds >= 0)
			{
				struct timeval now;
				struct timespec timeout;
				gettimeofday(&now, NULL);
				timeout.tv_sec = now.tv_sec + nseconds;
				timeout.tv_nsec = now.tv_usec * 1000;
				return pthread_cond_timedwait( &cond, &mutex.mutex, &timeout );
			}
			return pthread_cond_wait( &cond, &mutex.mutex );
		}
		int NotifyOne() { return pthread_cond_signal(&cond); }
		int NotifyAll() { return pthread_cond_broadcast(&cond); }
	};
}//namespace Thread
}//namespace GNET

#elif defined _REENTRANT_
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

namespace GNET
{
namespace Thread
{
	class Condition;
	class Mutex
	{
		friend class Condition;
		pthread_mutex_t mutex;
		Mutex(const Mutex& rhs);
		void Init(bool recursive)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE_NP : PTHREAD_MUTEX_ERRORCHECK_NP);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}
	public:
		~Mutex() 
		{
			while(pthread_mutex_destroy(&mutex) == EBUSY) { Lock(); UNLock(); }
		}
		explicit Mutex(bool recursive=false)
		{
			Init(recursive);
		}
		explicit Mutex(const char *id, bool recursive=false)
		{
			Init(recursive);
		}

		void Lock()   { pthread_mutex_lock(&mutex);   }
		void UNLock() { pthread_mutex_unlock(&mutex); }
		bool TryLock() { return pthread_mutex_trylock(&mutex) == 0; }

		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped () { mx->UNLock(); }
			explicit Scoped(Mutex& m) : mx(&m) { mx->Lock(); }
		};
	};

#if defined __GNUC__
	class SpinLock
	{
	#if (__GNUC__ == 4)
		pthread_spinlock_t spin;
		SpinLock(const SpinLock& rhs);
	public:
		~SpinLock() { pthread_spin_destroy(&spin); }
		explicit SpinLock(const char* id) { pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE); }
		void Lock() { pthread_spin_lock(&spin); }
		void UNLock() { pthread_spin_unlock(&spin); }
	#else
		volatile int locker;
		SpinLock(const SpinLock& rhs);
	public:
		~SpinLock() { }
		explicit SpinLock(const char *id) : locker(1) { }
		void Lock()
		{
			// XXX gcc 优化 BUG。会 coredump
			// 已确认版本 4.1.2 20070925 (Red Hat 4.1.2-33) 
			register int tmp;
			__asm__ __volatile__ (
				"1:		\n"
				"	cmp	$1, %0	\n"
				"	je	2f	\n"
				"	pause		\n"
				"	jmp	1b	\n"
				"2:		\n"
				"	xor	%1, %1	\n"
				"	xchg	%0, %1	\n"
				"	test	%1, %1	\n"
				"	je	1b	\n"
				: "=m"(locker), "=r"(tmp)
			);
		}
		void UNLock()
		{
			__asm__ __volatile__ (
				"	movl $1, %0		\n"
				: "=m"(locker)
			);
		}
	#endif
		class Scoped
		{
			SpinLock *sl;
		public:
			~Scoped () { sl->UNLock(); }
			explicit Scoped(SpinLock& m) : sl(&m) { sl->Lock(); }
		};
	};
#else
	typedef Mutex SpinLock;
#endif

	class RWLock
	{
		pthread_rwlock_t locker;
		RWLock(const RWLock& rhs);
	public:
		~RWLock()
		{
			while (pthread_rwlock_destroy(&locker) == EBUSY) { WRLock(); UNLock(); }
		}
		explicit RWLock() { pthread_rwlock_init(&locker, NULL); }
		void WRLock() { pthread_rwlock_wrlock(&locker); }
		void RDLock() { pthread_rwlock_rdlock(&locker); }
		void UNLock() { pthread_rwlock_unlock(&locker); }
		class WRScoped
		{
			RWLock *rw;
		public:
			~WRScoped () { rw->UNLock(); }
			explicit WRScoped(RWLock &l) : rw(&l) { rw->WRLock(); }
		};
		class RDScoped
		{
			RWLock *rw;
		public:
			~RDScoped () { rw->UNLock(); }
			explicit RDScoped(RWLock &l) : rw(&l) { rw->RDLock(); }
		};
	};

	class Condition
	{
		pthread_cond_t cond;
		Condition(const Condition& rhs);
	public:
		~Condition() 
		{
			while(pthread_cond_destroy(&cond) == EBUSY) { NotifyAll(); }
		}
		explicit Condition() { pthread_cond_init(&cond, NULL); }
		int Wait(Mutex & mutex) { return pthread_cond_wait(&cond, &mutex.mutex); }
		int TimedWait(Mutex & mutex, int nseconds)
		{
			if(nseconds >= 0)
			{
				struct timeval now;
				struct timespec timeout;
				gettimeofday(&now, NULL);
				timeout.tv_sec = now.tv_sec + nseconds;
				timeout.tv_nsec = now.tv_usec * 1000;
				return pthread_cond_timedwait(&cond, &mutex.mutex, &timeout);
			}
			return pthread_cond_wait(&cond, &mutex.mutex);
		}
		int NotifyOne() { return pthread_cond_signal(&cond); }
		int NotifyAll() { return pthread_cond_broadcast(&cond); }
	};

}//namespace Thread
}//namespace GNET

#else

namespace GNET
{
namespace Thread
{
	struct Mutex
	{
		~Mutex() { }
		explicit Mutex() {}
		explicit Mutex(const char *id, bool recursive=false) { }
		void Lock()   { }
		void UNLock() { }
		bool TryLock() { return true; }
		struct Scoped { ~Scoped() { } explicit Scoped(Mutex& m) { } };
	};
	typedef Mutex SpinLock;
	struct RWLock
	{
		~RWLock() { }
		void WRLock() { }
		void RDLock() { }
		void UNLock() { }
		RWLock() { }
		RWLock(const char* id) { }
		struct WRScoped { ~WRScoped() { } WRScoped(RWLock &) { } };
		struct RDScoped { ~RDScoped() { } RDScoped(RWLock &) { } };
	};
	struct Condition
	{
		~Condition() { } 
		explicit Condition( ) { }
		int Wait(Mutex & mutex) { return 0; }
		int TimedWait(Mutex & mutex, int nseconds) { return 0; }
		int NotifyOne() { return 0; }
		int NotifyAll() { return 0; }
	};
}//namespace Thread
}//namespace GNET
#endif

#endif//__MUTEX_H
