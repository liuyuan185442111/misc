#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <string>

template <typename T, typename Cmp = std::less<T>>
void insert_sort(T *a, int n, const Cmp &cmpor = Cmp())
{
	for(int i=1, j; i<n; ++i)
	{
		T picket = a[i];
		for(j=i-1; j>=0 && cmpor(picket,a[j]); --j)
			a[j+1] = a[j];
		a[j+1] = picket;
	}
}
template <typename T, typename Cmp = std::less<T>>
void insert_sort(T *a, T *b, const Cmp &cmpor = Cmp())
{
	if(std::distance(a,b) < 2) return;
	for(T *i=a+1, *j; i!=b; ++i)
	{
		T picket = *i;
		for(j=i-1; j!=a && cmpor(picket,*j); --j)
			*(j+1) = *j;
		*(j+1) = picket;
	}
}

template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp = std::less<ScoreType>>
class IntervalRankingList
{
	struct RankInfo
	{
		KeyType key;
		ScoreType score;
		InfoType other_info;
		int last_ranking;
	};
	ScoreCmp _cmpor;
	size_t _wanted;
	size_t _maxsize;
	RankInfo *_rank;
	RankInfo *_min;
	std::unordered_map<KeyType, RankInfo*> _hashMap;

	size_t _update_counter;
	std::string _data;
	bool _ready;
	time_t _last_reform_time;

	void update_min()
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			if(_cmpor(i->score, _min->score)) _min = i;
	}
	void update_min_and_map()
	{
		int rank = 0;
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
		{
			if(_cmpor(i->score, _min->score)) _min = i;
			i->last_ranking = ++rank;
			_hashMap[i->key] = i;
		}
	}
	//false: not changed
	bool sort()
	{
		if(_update_counter == 0) return false;
		if(_update_counter < _maxsize / 10 * 7)
			insert_sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		else
			std::stable_sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		_update_counter = 0;
		return true;
	}

public:
	void save()
	{
		int *buf = (int *)_rank - 1;
		*buf = (int)_hashMap.size();
		size_t len = sizeof(int) + sizeof(RankInfo) * *buf;
		_data.assign((const char *)buf, len);
		//send to db
	}

	void load(void *buf, size_t len)
	{
		if(_ready) return;
		if(buf && len)
		{
			int size = *(int *)buf;
			if(len != sizeof(int) + size * sizeof(RankInfo))
			{
				//err
				return;
			}
			std::memcpy((int *)_rank - 1, buf, len);
			sort();
			update_min_and_map();
			_data.assign((const char *)buf, len);
		}
		_ready = true;
	}
	void forward_merge(void *buf, size_t len)
	{
		if(buf && len)
		{
			int size = *(int *)buf;
			if(len != sizeof(int) + size * sizeof(RankInfo))
			{
				//err
				return;
			}
			for(RankInfo *i = (RankInfo *)((int *)buf + 1), *e = i + size; i != e; ++i)
				update(i->key, i->score, i->other_info);
		}
	}

	IntervalRankingList(size_t n) : _cmpor(ScoreCmp()), _wanted(n?n:1), _maxsize(_wanted + std::max(_wanted/10, (size_t)10)),
		_min(nullptr), _update_counter(0), _ready(false), _last_reform_time(0)
	{
		_rank = (RankInfo *)((int *)malloc(sizeof(int) + _maxsize * sizeof(RankInfo)) + 1);
		memset(_rank, 0, _maxsize * sizeof(RankInfo));
		_hashMap.reserve(_maxsize);
	}
	~IntervalRankingList() { free((int *)_rank - 1); }
	void reset()
	{
		_min = nullptr;
		_hashMap.clear();
		memset(_rank, 0, _maxsize * sizeof(RankInfo));
	}

	bool update(KeyType key, ScoreType score, const InfoType &info = InfoType())
	{
		update(key, score, [info](InfoType &in){in=info;});
	}
	//update score or info
	template <typename MAKE_INFO>
	bool update(KeyType key, ScoreType score, MAKE_INFO make_info)
	{
		if(!_ready) return false;
		auto it = _hashMap.find(key);
		if(it != _hashMap.end())
		{
			//found in _rank
			ScoreType old_score = it->second->score;
			it->second->score = score;
			make_info(it->second->other_info);
			if(it->second == _min)
			{
				if(_cmpor(old_score, score)) update_min();
				else _min->score = score;
			}
			else if(_cmpor(score, old_score) && _cmpor(score, _min->score))
			{
				_min = it->second;
			}
			++_update_counter;
		}
		else
		{
			size_t cursize = _hashMap.size();
			if(cursize != _maxsize)
			{
				//_rank not full
				_rank[cursize].key = key;
				_rank[cursize].score = score;
				make_info(_rank[cursize].other_info);
				_hashMap.insert(std::make_pair(key, _rank + cursize));
				if(!_min || _cmpor(score, _min->score))
				{
					_min = _rank + cursize;
				}
				++_update_counter;
			}
			else if(_cmpor(_min->score, score))
			{
				//substitute
				_hashMap.erase(_min->key);
				_min->key = key;
				_min->score = score;
				make_info(_min->other_info);
				_hashMap.insert(std::make_pair(key, _min));
				update_min();
				++_update_counter;
			}
		}
		return true;
	}
	bool remove(KeyType key)
	{
		if(!_ready) return false;
		auto it = _hashMap.find(key);
		if(it == _hashMap.end()) return false;
		RankInfo *target = it->second;
		_hashMap.erase(key);

		RankInfo *last = _rank + _hashMap.size();
		if(target == last)
		{
			if(_rank == last)
			{
				_min = nullptr;
			}
			else if(target == _min)
			{
				--_min;
				update_min();
			}
		}
		else
		{
			_hashMap[last->key] = target;
			*target = *last;
			if(target == _min)
			{
				update_min();
			}
		}
		//不通知gs问题也不大
		++_update_counter;
		return true;
	}

	bool reform()
	{
		if(!_ready) return false;
		if(sort())
		{
			save();
			update_min_and_map();
		}
		_last_reform_time = time(nullptr);
		return true;
	}

	//名单满的时候返回最小分数
	//用于提前做判断剔除榜外数据
	ScoreType get_min_score()
	{
		if(_min && _maxsize == _hashMap.size()) return _min->score;
		return ScoreType();
	}
	time_t get_last_reform_time()
	{
		return _last_reform_time;
	}
	void get_all_data(std::string &data)
	{
		data = _data;
	}
	void get_wanted_data(std::string &data)
	{
	}

	void dump(std::ostream &out = std::cout)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << ", last_ranking:" << i->last_ranking << std::endl;
	}
};

template <typename InfoType>
InfoType &make_info(InfoType &info) { return info; }

int topn()
{
	srand(time(nullptr));
	srand(0);
	IntervalRankingList<int, int, int> r(100);
	r.load(nullptr, 0);
	for(int i=1000000;i>0;--i)
	{
		if(i%100 == 0) r.reform();
		int t = rand();
		//r.update(t%10000,t,make_info<int>);
		r.update(t%10000,t);
		if(rand()&1) r.remove(t%10000);
	}
	r.reform();
	r.dump(std::cout);
	std::cout << std::endl;

	srand(0);
	IntervalRankingList<int, int, int> s(100);
	s.load(nullptr, 0);
	for(int i=10000;i>0;--i)
	{
		int t = rand();
		//r.update(t%10000,t,make_info<int>);
		s.update(t%10000,t);
	}
	s.reform();
	s.dump();
	std::cout << std::endl;

	std::string data;
	s.get_all_data(data);
	r.forward_merge((void *)data.c_str(), data.length());
	r.reform();
	r.dump(std::cout);
	return 0;
}

using std::string;
using std::cout;
using std::endl;

#include <sys/time.h>
int main()
{
	topn();
	return 0;
}

