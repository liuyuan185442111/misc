#ifndef QUEUEWITHWEIGHT_H
#define QUEUEWITHWEIGHT_H

#include <vector>
#include <list>
#include <map>
#include <set>

using TIMETYPE = time_t;

//可用于游戏排队
template <typename T>
class QueueWithWeight
{
	using ListNode = typename std::list<T>::iterator;
	struct State
	{
		unsigned index;
		ListNode node;
		TIMETYPE offlinetime = 0;
		State(unsigned i, ListNode n) : index(i), node(n) {}
	};
	std::map<T, State> _all;
	std::vector<unsigned> _weights;
	std::vector<std::list<T>> _queues;
public:
	QueueWithWeight(const std::vector<unsigned> &weights={})
	{
		if(weights.empty())
		{
			_queues.emplace_back(std::queue<T>());
			return;
		}
		_weights = weights;
		_queues.assign(weights.size(), std::queue<T>());
	}
	bool push(T t, unsigned index=0)
	{
		if(index >= _queues.size())
			return false;
		leave(t);
		_queues[index].push_back(t);
		_all.emplace(t, index, --_queues[index].end());
		return true;
	}
	bool empty()
	{
		for(const auto &q : _queues)
		{
			if(!q.empty())
				return false;
		}
		return true;
	}
	T pop()
	{
		if(_queues.size() == 1)
		{
			T t = _queues[0]->front();
			_queues[0]->pop_front();
			_all.erase(t);
			return t;
		}
		std::vector<unsigned> weights(_weights);
		unsigned weight_sum=0;
		for(int i=_queues.size(); i; --i)
		{
			if(_queues[i-1].empty())
				weights[i-1] = 0;
			else
				weight_sum += weights[i-1];
		}
		int pos = rand() % weight_sum;
		for(int i=weights.size(); i; --i)
		{
			if(weights[i-1] > pos)
			{
				T t = _queues[i-1].front();
				_queues[i-1].pop_front();
				_all.erase(t);
				return t;
			}
			pos -= weights[i-1];
		}
		return 0;
	}
	void offline()
	{
	}
	void leave(T t)
	{
		auto iter = _all.find(t);
		if(iter == _all.end())
			return;
		_queues[iter->index].erase(iter->node);
		_all.erase(iter);
	}
};

#endif // QUEUEWITHWEIGHT_H
