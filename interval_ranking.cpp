//.h
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

template <typename KeyType, typename ScoreType, typename ScoreCmp = std::less<ScoreType>>
class IntervalRankingList
{
	struct RankInfo
	{
		KeyType key;
		ScoreType score;
		int last_ranking;
	};

	ScoreCmp _cmpor;
	size_t _maxsize;
	RankInfo *_rank;
	RankInfo *_min;
	std::unordered_map<KeyType, RankInfo*> _hashMap;

	bool _ready;
	size_t _update_counter;
	time_t _last_reform_time;
	std::string _data;

	void update_min()
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			if(_cmpor(i->score, _min->score)) _min = i;
	}
	void update_min_and_rank()
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
	bool try_stable_sort()
	{
		if(_update_counter == 0) return false;
		if(_update_counter < _maxsize / 4 * 3)
			insert_sort     (_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		else
			std::stable_sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		_update_counter = 0;
		return true;
	}

public:
	IntervalRankingList(size_t n) : _cmpor(ScoreCmp()), _maxsize(n?n:1),
		_min(nullptr), _ready(false), _update_counter(0), _last_reform_time(0)
	{
		_rank = (RankInfo *)malloc(_maxsize * sizeof(RankInfo));
		for(RankInfo *p = _rank, *e = _rank + _maxsize; p != e; ++p)
		{
			new(p) RankInfo();
		}
		_hashMap.reserve(_maxsize);
	}
	~IntervalRankingList()
	{
		for(RankInfo *p = _rank, *e = _rank + _maxsize; p != e; ++p)
		{
			p->~RankInfo();
		}
		free(_rank);
	}
	void reset()
	{
		_min = nullptr;
		_hashMap.clear();
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
			//db --> _rank --> data
			//careful about size == 0
			try_stable_sort();
			update_min_and_rank();
			_last_reform_time = time(nullptr);
		}
		_ready = true;
	}
	void save()
	{
		//_rank --> data --> db
	}

	bool update(KeyType key, ScoreType score);
	bool remove(KeyType key);
	bool reform();

	void dump(std::ostream &out = std::cout)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << ", last_ranking:" << i->last_ranking << std::endl;
	}
};


//.tcc
template <typename KeyType, typename ScoreType, typename ScoreCmp>
bool IntervalRankingList<KeyType, ScoreType, ScoreCmp>::update(KeyType key, ScoreType score)
{
	if(!_ready) return false;
	auto it = _hashMap.find(key);
	if(it != _hashMap.end())
	{
		//found in _rank
		ScoreType old_score = it->second->score;
		it->second->score = score;
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
			_hashMap.insert(std::make_pair(key, _min));
			update_min();
			++_update_counter;
		}
	}
	return true;
}

template <typename KeyType, typename ScoreType, typename ScoreCmp>
bool IntervalRankingList<KeyType, ScoreType, ScoreCmp>::remove(KeyType key)
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
	++_update_counter;
	return true;
}

template <typename KeyType, typename ScoreType, typename ScoreCmp>
bool IntervalRankingList<KeyType, ScoreType, ScoreCmp>::reform()
{
	if(!_ready) return false;
	if(try_stable_sort())
	{
		save();
		update_min_and_rank();
	}
	_last_reform_time = time(nullptr);
	return true;
}


//test.cpp
#include <sys/time.h>
using std::cout;
using std::endl;
using std::string;
int test()
{
	srand(time(nullptr));
	srand(0);
	IntervalRankingList<int, int> r(100);
	r.load(nullptr, 0);
	for(int i=1000000;i>0;--i)
	{
		if(i%100 == 0) r.reform();
		int t = rand();
		r.update(t%10000,t);
		if(rand()&1) r.remove(t%10000);
	}
	r.reform();
	r.dump(std::cout);
	std::cout << std::endl;

	srand(0);
	IntervalRankingList<int, int> s(100);
	s.load(nullptr, 0);
	for(int i=10000;i>0;--i)
	{
		int t = rand();
		s.update(t%10000,t);
	}
	s.reform();
	s.dump();
	std::cout << std::endl;
	return 0;
}

int main()
{
	test();
	return 0;
}

