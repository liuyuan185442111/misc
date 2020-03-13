#ifndef L_MESSAGE_H_
#define L_MESSAGE_H_

#include <stdlib.h>
#include <string.h>
#include <vector>
#include "msg.h"
#include "itimer.h"
#include "thread.h"
#include "mutex.h"

class MsgQueue : public GNET::Thread::Runnable
{
	typedef std::vector<MSG> MSGQUEUE;
	MSGQUEUE _queue;
	friend class MsgQueueList;
	void AddTask() { GNET::Thread::AddTask(this); }
	virtual void Run() { Send(); delete this; }
public:
	size_t Size() { return _queue.size(); }
	bool IsEmpty() { return _queue.empty(); }
	void Swap(MsgQueue &q) { _queue.swap(q._queue); }
	void AddMsg(const MSG &msg) { _queue.push_back(msg); }
	void Send();
};

class MsgQueueList : public GNET::IntervalTimer::Observer
{
	enum
	{
		MSG_SIZE = 256,
		LIST_SIZE = 256
	};
	MsgQueue * 		_list[LIST_SIZE];//for延时消息
	int 	   		_offset;
	MsgQueue 		_cur_queue;
	GNET::Thread::Mutex 	_lock_list;//for _list
	GNET::Thread::Mutex 	_lock_cur;//for _cur_queue

	MsgQueue *_GetQueue(int target)
	{
		if(_list[target] == NULL)
			_list[target] = new MsgQueue();
		return _list[target];
	}
	inline void _SendCurQueue()
	{
		MsgQueue *pQueue = new MsgQueue;
		pQueue->Swap(_cur_queue);
		pQueue->AddTask();
	}
	virtual bool Update()
	{
		_lock_cur.Lock();
		if(!_cur_queue.IsEmpty())
			_SendCurQueue();
		_lock_cur.UNLock();

		GNET::Thread::Mutex::Scoped keeper(_lock_list);
		if(_list[_offset])
		{
			_list[_offset]->AddTask();
			_list[_offset] = NULL;
		}
		if(++_offset == LIST_SIZE) _offset = 0;
		return true;
	}
public:
	MsgQueueList() : _offset(0)
	{
		memset(_list, 0, sizeof(_list));
	}
	~MsgQueueList()
	{
		for(int i=0; i<LIST_SIZE; ++i)
		{
			if(_list[i]) delete _list[i];
		}
	}
	void AddMsg(const MSG &msg)
	{
		GNET::Thread::Mutex::Scoped keeper(_lock_cur);
		_cur_queue.AddMsg(msg);
		if(_cur_queue.Size() < MSG_SIZE) return;
		_SendCurQueue();
	}
	void SendCurQueue()
	{
		_lock_cur.Lock();
		if(_cur_queue.IsEmpty()) return;
		MsgQueue queue;
		queue.Swap(_cur_queue);
		_lock_cur.UNLock();
		queue.Send();
	}
	//延迟latency个tick后发消息
	void AddMsg(const MSG &msg, size_t latency)
	{
		if(latency >= LIST_SIZE) return;
		GNET::Thread::Mutex::Scoped keeper(_lock_list);
		int target = _offset + latency;
		if(target >= LIST_SIZE) target -= LIST_SIZE;
		_GetQueue(target)->AddMsg(msg);
	}
};

#endif // L_MESSAGE_H_
