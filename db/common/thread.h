#ifndef _L_THREAD_H
#define _L_THREAD_H

#include <stddef.h>//for size_t
#include <map>

namespace lcore {

class Runnable
{
protected:
	int _priority; //只在未定义_REENTRANT_时有效, 越小优先级越高
public:
	Runnable(int priority = 1) : _priority(priority) { }
	virtual ~Runnable() { }
	virtual void Run() = 0;
	void SetPriority(int priority) { _priority = priority; }
	int GetPriority() const { return _priority; }
};

namespace ThreadPool {
//task should delete itself in Run() when needed
void AddTask(Runnable *task);
//pool_size: 线程数量, 仅在定义_REENTRANT_时有效
void Start(size_t pool_size = 4, bool daemon = false);
void Stop();
//排队中的任务数量
size_t TaskSize();
}

} //namespace lcore

#endif
