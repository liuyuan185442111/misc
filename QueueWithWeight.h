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
	TIMETYPE avgwaittime(unsigned index=0);
	//获取本tick需要广播的数据, int为在本队列中的位次
	//应以小于1秒的间隔周期调用
	void tick(std::vector<std::vector<std::pair<T,int>>> notice);

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

	struct Config
	{
		int upbound;
		int interval;
		int num;
		TIMETYPE last_hit_time = 0;
		bool hit;
	};
	std::vector<Config> _config;
	TIMETYPE _last_tick_time = 0;
	std::map<TIMETYPE, std::vector<std::vector<std::pair<T,int>>>> _notice;

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
		return 0;
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

	_config.assign(2, Config());
	_config[0].upbound = 3;
	_config[0].interval = 2;
	_config[1].upbound = 14;
	_config[1].interval = 10;

	if(!_config.empty())
		_config[0].num = _config[0].upbound;
	for(size_t i=1; i<_config.size(); ++i)
	{
		_config[i].num = _config[i].upbound - _config[i-1].upbound;
	}
}
template <typename T>
size_t QueueWithWeight<T>::push(T t, unsigned index)
{
	if(index >= (unsigned)_queues.size())
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
	unsigned pos = rand() % weight_sum;
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
TIMETYPE QueueWithWeight<T>::avgwaittime(unsigned index)
{
	return 1;
}
template <typename T>
void QueueWithWeight<T>::tick(std::vector<std::vector<std::pair<T,int>>> notice)
{
	static TIMETYPE curtime = 100;
	++curtime;
	//auto curtime = _gettime();

	printf("QueueWithWeight::tick %lld\n", curtime);
	auto n = _getnum();
	while(n--)
	{
		T t = pop();
		if(t)
		{
			printf("pop %u\n", t);
		}
		else break;
	};


	bool hit_any = false;
	for(auto &cfg : _config)
	{
		if(curtime - cfg.last_hit_time >= cfg.interval)
		{
			cfg.hit = true;
			cfg.last_hit_time = curtime;
			hit_any = true;
		}
		else
		{
			cfg.hit = false;
		}
	}
	if(!hit_any) return;

	int upbound;
	for(auto iter = _config.crbegin(); iter != _config.crend(); ++iter)
	{
		if(iter->hit)
		{
			upbound = iter->upbound;
			break;
		}
	}
	cout << "upbound is " << upbound << endl;

	for(size_t index = 0; index < _queues.size(); ++index)
	{
		auto &queue = _queues[index];
		if(queue.empty()) continue;
		auto iter = queue.begin();
		int counter = 0;
		for(const auto &cfg : _config)
		{
			if(cfg.upbound > upbound) break;
			if(cfg.hit)
			{
				//待优化
				int diff = 0;
				int circle = cfg.num/cfg.interval;
				for(; counter < cfg.upbound; ++counter)
				{
					if(--circle == 0) ++diff;
					for(int k = _notice[curtime+diff].size(); k < index; ++k)
						_notice[curtime+diff].push_back(std::vector<std::pair<T,int>>());
					_notice[curtime+diff][index].emplace_back(*iter, counter);
					++iter;
					if(iter == queue.end()) break;
				}
			}
			else
			{
				if(counter + cfg.num < queue.size())
				{
					std::advance(iter, cfg.num);
					counter += cfg.num;
				}
				else
				{
					//待优化
					for(; counter < cfg.upbound; ++counter)
					{
						++iter;
						if(iter == queue.end()) break;
					}
				}
			}
		}
	}


	notice.clear();
	if(_notice.empty()) return;
	if(curtime >= _notice.begin()->first)
	{
		notice = std::move(_notice.begin()->second);
		_notice.erase(_notice.begin());
	}
}

#endif // QUEUEWITHWEIGHT_H
