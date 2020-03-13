#ifndef __THREAD_H
#define __THREAD_H

#include <stddef.h>//for size_t
#include <map>

namespace GNET
{
namespace Thread
{

//priority暂只在未定义_REENTRANT_时生效
//priority越小优先级越高
class Runnable
{
protected:
	int m_priority;
public:
	Runnable(int priority = 1) : m_priority(priority) { }
	virtual ~Runnable() { }
	virtual void Run() = 0;
	void SetPriority(int priority) { m_priority = priority; }
	int GetPriority() const { return m_priority; }
};

class Policy
{
protected:
	size_t m_max_task_size;
	bool   m_will_quit;
public:
	Policy(int max_task_size = 2048) : m_max_task_size(max_task_size), m_will_quit(false) { }
	virtual ~Policy() { }
	virtual Policy *Clone() const { return new Policy(*this); }

	void SetMaxTaskSize(size_t max_task_size) { m_max_task_size = max_task_size; }
	void SetQuit() { m_will_quit = true; }
	bool GetQuit() { return m_will_quit; }

	virtual bool CanAddTask(Thread::Runnable *pTask, size_t queueing_tasksize, bool bForced)
	{
		return !m_will_quit && (queueing_tasksize < m_max_task_size || bForced);
	}
};

//默认使用Policy, 如需修改请派生Policy并调用该函数
void SetPolicy(const Policy &policy);
//task should delete itself in Run() when needed
void AddTask(Runnable *task, bool bForced = false);
//pool_size: 线程数量, 仅在定义_REENTRANT_时生效
//收到SIGUSR1信号后停止
void Start(size_t pool_size = 4, bool daemon = false);
//排队中的任务数量
size_t TaskSize();
//实际线程数量
size_t ThreadSize();

}
}

#endif//__THREAD_H
