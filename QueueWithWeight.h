#ifndef QUEUEWITHWEIGHT_H
#define QUEUEWITHWEIGHT_H

#include <vector>
#include <list>
#include <map>
#include <set>

using TIMETYPE = time_t;
//掉线保留时间
static constexpr int DEFAULT_OVERTIME = 30;

//可用于游戏排队, 支持取消排队, 掉线重入
//假定T为正数有效
template <typename T>
class QueueWithWeight
{
public:
	QueueWithWeight(const std::vector<unsigned> &weights={});
	size_t push(T t, unsigned index=0);
	bool empty();
	size_t size();
	T pop();
	void offline(T t);
	void leave(T t);
	void tick();

private:
	using Queue = std::list<T>;
	using QueueNode = typename Queue::iterator;
	struct State
	{
		unsigned index;
		QueueNode node;
		TIMETYPE offtime = 0;
		State(unsigned i, QueueNode n) : index(i), node(n) {}
		State() = default;
		State &operator=(State &&) = default;
	};
	std::map<T, State> _all;
	std::vector<unsigned> _weights;
	std::vector<Queue> _queues;

	T _pop_from_queue(int index)
	{
		T t;
		auto &q = _queues[index];
		do
		{
			t = q.front();
			q.pop_front();
			auto iter = _all.find(t);
			if(iter != _all.end())
			{
				if(iter->second.offtime == 0)
				{
					_all.erase(iter);
					return t;
				}
				//offline
				_all.erase(iter);
			}
			else return t;
		} while(!q.empty());
		return 0;
	}

	//portable
	TIMETYPE _gettime() const
	{
		return time(nullptr);
	}
	unsigned _getnum() const
	{
		return 1;
	}
};

template <typename T>
QueueWithWeight<T>::QueueWithWeight(const std::vector<unsigned> &weights)
{
	if(weights.size() < 2)
	{
		_queues.emplace_back(Queue());
	}
	else
	{
		_weights = weights;
		_queues.assign(weights.size(), Queue());
	}
}
template <typename T>
size_t QueueWithWeight<T>::push(T t, unsigned index)
{
	if(index >= _queues.size())
		return 0;
	auto iter = _all.find(t);
	State *pval;
	if(iter != _all.end())
	{
		if(iter->second.offtime > 0 && _gettime() < iter->second.offtime + DEFAULT_OVERTIME)
		{
			iter->second.offtime = 0;
			return _queues[iter->second.index].size();
		}
		pval = &iter->second;
	}
	else
	{
		pval = &_all[t];
	}
	_queues[index].push_back(t);
	*pval = State(index, --_queues[index].end());
	return _queues[index].size();
}
template <typename T>
bool QueueWithWeight<T>::empty()
{
	for(const auto &q : _queues)
	{
		if(!q.empty())
			return false;
	}
	return true;
}
template <typename T>
size_t QueueWithWeight<T>::size()
{
	size_t s = 0;
	for(const auto &q : _queues)
	{
		s += q.size();
	}
	return s;
}
template <typename T>
T QueueWithWeight<T>::pop()
{
	if(empty()) return 0;
	if(_queues.size() == 1)
	{
		T t = _pop_from_queue(0);
		return t>0 ? t : pop();
	}
	std::vector<unsigned> weights(_weights);
	unsigned weight_sum = 0;
	for(auto i=_queues.size(); i; --i)
	{
		if(_queues[i-1].empty())
			weights[i-1] = 0;
		else
			weight_sum += weights[i-1];
	}
	int pos = rand() % weight_sum;
	for(auto i=weights.size(); i; --i)
	{
		if(weights[i-1] > pos)
		{
			T t = _pop_from_queue(i-1);
			return t>0 ? t : pop();
		}
		pos -= weights[i-1];
	}
	return 0;
}
template <typename T>
void QueueWithWeight<T>::offline(T t)
{
	auto iter = _all.find(t);
	if(iter != _all.end())
		iter->second.offtime = _gettime();
}
template <typename T>
void QueueWithWeight<T>::leave(T t)
{
	auto iter = _all.find(t);
	if(iter != _all.end())
	{
		_queues[iter->second.index].erase(iter->second.node);
		_all.erase(iter);
	}
}
template <typename T>
void QueueWithWeight<T>::tick()
{
	unsigned n = _getnum();
	while(n--)
	{
		T t = pop();
		if(t)
		{
			printf("pop %u\n", t);
		}
		else break;
	};
}

#endif // QUEUEWITHWEIGHT_H
