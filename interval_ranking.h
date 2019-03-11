#ifndef INTERVAL_RANKING_H
#define INTERVAL_RANKING_H

#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstring>

namespace grank
{

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

	size_t _update_counter;
	time_t _last_reform_time;

	void update_min()
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			if(_cmpor(i->score, _min->score)) _min = i;
	}
	void update_min_and_rank();
	//return false: not changed from last sort
	bool try_stable_sort();

public:
	IntervalRankingList(size_t n) : _cmpor(ScoreCmp()), _maxsize(n?n:1),
		_min(nullptr), _update_counter(0), _last_reform_time(0)
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

	void load(void *buf, size_t len);
	void save();

	bool update(KeyType key, ScoreType score);
	bool remove(KeyType key);
	bool reform();

	void dump(std::ostream &out = std::cout)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << ", last_ranking:" << i->last_ranking << std::endl;
	}
};



template <typename KeyType, typename ScoreType, typename ScoreCmp>
void IntervalRankingList<KeyType, ScoreType, ScoreCmp>::load(void *buf, size_t len)
{
	if(buf && len)
	{
		int size = *(int *)buf;
		if(len != sizeof(int) + size * sizeof(RankInfo))
		{
			return;
		}
		if(size > 0)
		{
			RankInfo *src = (RankInfo *)((int *)buf + 1);
			RankInfo *dst = _rank;
			int i = 0;
			for(; i<size && i<_maxsize; ++i,++src,++dst)
			{
				*dst = *src;
				_hashMap.insert(std::make_pair(dst->key, dst));
			}
			for(; i<size; ++i,++src)
				update(src->key, src->score);
			try_stable_sort();
			save();
			update_min_and_rank();
			_last_reform_time = time(nullptr);
		}
	}
}

template <typename KeyType, typename ScoreType, typename ScoreCmp>
void IntervalRankingList<KeyType, ScoreType, ScoreCmp>::save()
{
}

template <typename KeyType, typename ScoreType, typename ScoreCmp>
void IntervalRankingList<KeyType, ScoreType, ScoreCmp>::update_min_and_rank()
{
	int rank = 0;
	for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
	{
		if(_cmpor(i->score, _min->score)) _min = i;
		i->last_ranking = ++rank;
		_hashMap[i->key] = i;
	}
}

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
		for(j=i-1; j>=a && cmpor(picket,*j); --j)
			*(j+1) = *j;
		*(j+1) = picket;
	}
}

template <typename KeyType, typename ScoreType, typename ScoreCmp>
bool IntervalRankingList<KeyType, ScoreType, ScoreCmp>::try_stable_sort()
{
	if(_update_counter == 0) return false;
	if(_update_counter < _maxsize / 4 * 3)
		insert_sort     (_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
	else
		std::stable_sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
	_update_counter = 0;
	return true;
}

template <typename KeyType, typename ScoreType, typename ScoreCmp>
bool IntervalRankingList<KeyType, ScoreType, ScoreCmp>::update(KeyType key, ScoreType score)
{
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
	if(try_stable_sort())
	{
		save();
		update_min_and_rank();
	}
	_last_reform_time = time(nullptr);
	return true;
}

}
#endif

