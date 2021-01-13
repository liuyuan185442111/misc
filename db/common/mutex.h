#ifndef _L_MUTEX_H
#define _L_MUTEX_H

#ifdef _REENTRANT_

#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

namespace lcore {
class Condition;
class Mutex
{
	friend class Condition;
	pthread_mutex_t mutex;
	Mutex(const Mutex& rhs);
public:
	~Mutex() { while(pthread_mutex_destroy(&mutex) == EBUSY) { Lock(); UNLock(); } }
	Mutex() { pthread_mutex_init(&mutex, NULL); }

	void Lock()    { pthread_mutex_lock(&mutex); }
	void UNLock()  { pthread_mutex_unlock(&mutex); }
	bool TryLock() { return pthread_mutex_trylock(&mutex) == 0; }

	class Scoped
	{
		Mutex *mx;
	public:
		~Scoped () { mx->UNLock(); }
		explicit Scoped(Mutex &m) : mx(&m) { mx->Lock(); }
	};
};

#ifdef __GNUC__
class SpinLock
{
#if (__GNUC__ == 4)
	pthread_spinlock_t spin;
	SpinLock(const SpinLock& rhs);
public:
	~SpinLock() { pthread_spin_destroy(&spin); }
	SpinLock() { pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE); }
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
	~RWLock() { while(pthread_rwlock_destroy(&locker) == EBUSY) { WRLock(); UNLock(); } }
	RWLock() { pthread_rwlock_init(&locker, NULL); }
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
	~Condition() { while(pthread_cond_destroy(&cond) == EBUSY) { NotifyAll(); } }
	Condition() { pthread_cond_init(&cond, NULL); }
	int Wait(Mutex &mutex) { return pthread_cond_wait(&cond, &mutex.mutex); }
	int TimedWait(Mutex &mutex, int nseconds)
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

#else

namespace lcore {
struct Mutex
{
	~Mutex() { }
	Mutex() { }
	void Lock() { }
	void UNLock() { }
	bool TryLock() { return true; }
	struct Scoped { ~Scoped() { } explicit Scoped(Mutex& m) { } };
};
typedef Mutex SpinLock;
struct RWLock
{
	~RWLock() { }
	RWLock() { }
	void WRLock() { }
	void RDLock() { }
	void UNLock() { }
	struct WRScoped { ~WRScoped() { } WRScoped(RWLock &) { } };
	struct RDScoped { ~RDScoped() { } RDScoped(RWLock &) { } };
};
struct Condition
{
	~Condition() { } 
	Condition() { }
	int Wait(Mutex &mutex) { return 0; }
	int TimedWait(Mutex &mutex, int nseconds) { return 0; }
	int NotifyOne() { return 0; }
	int NotifyAll() { return 0; }
};

#endif

} //namespace lcore

#endif //_L_MUTEX_H
