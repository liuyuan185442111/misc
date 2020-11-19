#ifndef QUEUEWITHWEIGHT_H
#define QUEUEWITHWEIGHT_H

#include <vector>
#include <list>
#include <map>
#include <assert.h>

using TIMETYPE = time_t;
//掉线保留时间
static constexpr int DEFAULT_OVERTIME = 30;
//开始进行出队时间统计的队伍大小
static constexpr int TIME_STATISTICS_THRESHOLD = 20;

//可用于单线程游戏排队, 支持取消排队, 掉线重入
//假定T为正数有效
template <typename T>
class QueueWithWeight
{
public:
	QueueWithWeight(const std::vector<unsigned> &weights={});
	//返回的是所在队列的人数
	size_t push(T t, unsigned index=0);
	bool empty();
	size_t size();
	T pop();
	void offline(T t);
	void leave(T t);
	TIMETYPE avgwaittime(unsigned index=0);
	//获取本tick需要广播的数据, int为在本队列中的从0开始的位次
	//应以小于1秒的间隔周期调用
	void tick(std::vector<std::vector<std::pair<T,int>>> &notice);

private:
	using QueueNode = typename std::list<T>::iterator;

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

	struct Queue
	{
		std::list<T> queue;
		unsigned weight;
		int size = 0;
		bool empty() const { return queue.empty(); }
		T pop()
		{
			T t = queue.front();
			queue.pop_front();
			--size;
			return t;
		}
		size_t push(T t)
		{
			queue.push_back(t);
			return ++size;
		}
		void erase(const typename std::list<T>::iterator &iter)
		{
			queue.erase(iter);
			--size;
		}
	};
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
	std::map<TIMETYPE, std::vector<std::vector<std::pair<T,int>>>> _notice;

	struct TimeStat
	{
		TIMETYPE first_blood = 0;
		int sum_time;
		int sum_count;
		TIMETYPE begin_time;
		int cur_count;
	};
	std::vector<TimeStat> _time_stat;

	T _pop_from_queue(int index)
	{
		T t;
		auto &q = _queues[index];
		do
		{
			t = q.pop();
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
	void _tick(TIMETYPE curtime)
	{
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

		for(size_t index=0; index<_queues.size(); ++index)
		{
			const auto &queue = _queues[index];
			if(queue.empty()) continue;

			auto iter = queue.queue.begin();
			int pos = 0;
			for(const auto &cfg : _config)
			{
				if(cfg.upbound > upbound) break;
				if(cfg.hit)
				{
					int diff = 0;
					auto *pnotice = &_notice[curtime+diff];
					int circle0 = std::ceil((float)cfg.num/cfg.interval);
					int circle = circle0;
					//cout << "circle is " << cfg.num << "/" << cfg.interval << "=" << circle << endl;
					int maxpos = std::min(cfg.upbound, (int)queue.size);
					for(; pos<maxpos; ++pos,++iter)
					{
						//cout << "pos " << pos << ", val " << *iter << ", diff " << diff << endl;
						for(int k=pnotice->size(); k<=index; ++k)
							pnotice->push_back(std::vector<std::pair<T,int>>());
						(*pnotice)[index].emplace_back(*iter, pos);
						if(--circle == 0)
						{
							circle = circle0;
							++diff;
							pnotice = &_notice[curtime+diff];
						}
					}
					if(maxpos < cfg.upbound) break;
				}
				else
				{
					if((int)queue.size < cfg.upbound) break;
					std::advance(iter, cfg.num);
					pos = cfg.upbound;
				}
			}
		}
	}

	//portable
	TIMETYPE _gettime() const
	{
		return time(nullptr);
	}
	unsigned _getnum() const
	{
		return 10;
	}
};

template <typename T>
QueueWithWeight<T>::QueueWithWeight(const std::vector<unsigned> &weights)
{
	if(weights.size() < 2)
	{
		_queues.emplace_back(Queue());
		_time_stat.push_back(TimeStat());
	}
	else
	{
		_queues.assign(weights.size(), Queue());
		for(size_t i=0; i<weights.size(); ++i)
			_queues[i].weight = weights[i];
		_time_stat.assign(weights.size(), TimeStat());
	}

	_config.assign(5, Config());
	_config.at(0).upbound = 200;
	_config.at(0).interval = 2;
	_config.at(1).upbound = 750;
	_config.at(1).interval = 5;
	_config.at(2).upbound = 2000;
	_config.at(2).interval = 12;
	_config.at(3).upbound = 5000;
	_config.at(3).interval = 30;
	_config.at(4).upbound = 10000;
	_config.at(4).interval = 60;

	if(!_config.empty())
		_config[0].num = _config[0].upbound;
	for(size_t i=1; i<_config.size(); ++i)
	{
		_config[i].num = _config[i].upbound - _config[i-1].upbound;
	}
	for(const auto &cfg : _config)
		assert(cfg.num > 0);
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
		if(iter->second.offtime == 0) return 0;
		if(_gettime() < iter->second.offtime + DEFAULT_OVERTIME)
		{
			//断线重入
			iter->second.offtime = 0;
			return _queues[iter->second.index].size/2+1;
		}
		else
		{
			//断线超时
			_queues[iter->second.index].erase(iter->second.node);
		}
		pval = &iter->second;
	}
	else
	{
		pval = &_all[t];
	}
	if(_queues[index].push(t) == 1)
		_time_stat[index].first_blood = _gettime();
	*pval = State(index, --_queues[index].queue.end());
	return _queues[index].size;
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
	for(auto q : _queues)
	{
		s += q.size;
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
	std::vector<unsigned> weights(_queues.size());
	for(int i=0; i<weights.size(); ++i)
		weights[i] = _queues[i].weight;
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
void QueueWithWeight<T>::tick(std::vector<std::vector<std::pair<T,int>>> &notice)
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

	_tick(curtime);

	notice.clear();
	if(_notice.empty()) return;
	if(curtime >= _notice.begin()->first)
	{
		notice = std::move(_notice.begin()->second);
		_notice.erase(_notice.begin());
	}
}

#endif // QUEUEWITHWEIGHT_H
