#include <cstddef>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>
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

template <typename KeyType, typename InfoType, typename ScoreType, typename ScoreCmp=std::less<ScoreType>>
class RankingList
{
	struct RankInfo
	{
		KeyType key;
		ScoreType score;
		InfoType other_info;
		int last_ranking;
		RankInfo(KeyType _key, ScoreType _score, const InfoType &_info)
			: key(_key), score(_score), other_info(_info), last_ranking(0) { }
	};
	ScoreCmp _cmpor;
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
			std::sort  (_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		_update_counter = 0;
		return true;
	}
	void save()
	{
		int *buf = (int *)_rank - 1;
		*buf = (int)_hashMap.size();
		size_t len = sizeof(int) + sizeof(RankInfo) * *buf;
		_data.assign((const char *)buf, len);
		//send to db
	}

public:
	void load(void *buf, size_t len)
	{
		if(buf && len)
		{
			int size = *(int *)buf;
			if(len != size * sizeof(RankInfo))
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

	RankingList(size_t n) : _cmpor(ScoreCmp()), _maxsize(n?n:1), _min(nullptr), _update_counter(0), _ready(false), _last_reform_time(0)
	{
		_rank = (RankInfo *)((int *)malloc(sizeof(int) + n * sizeof(RankInfo)) + 1);
		_hashMap.reserve(_maxsize);
	}
	~RankingList() { free((int *)_rank - 1); }

	bool update(KeyType key, ScoreType score, const InfoType &info = InfoType())
	{
		if(!_ready) return false;
		auto it = _hashMap.find(key);
		if(it != _hashMap.end())
		{
			//found in _rank
			it->second->score = score;
			it->second->other_info = info;
			if(it->second == _min)
			{
				update_min();
			}
			++_update_counter;
		}
		else
		{
			size_t cursize = _hashMap.size();
			if(cursize != _maxsize)
			{
				//_rank not full
				_hashMap.insert(std::make_pair(key, &(_rank[cursize] = RankInfo(key, score, info))));
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
				*_min = RankInfo(key, score, info);
				_hashMap.insert(std::make_pair(key, _min));
				update_min();
				++_update_counter;
			}
		}
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
		_last_reform_time = time(NULL);
		return true;
	}

	void dump(std::ostream &out)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << ", last_ranking:" << i->last_ranking << std::endl;
	}
};


typedef RankingList<int, int, int> rank;
int main()
{
	//srand(time(NULL));
	srand(10010);
	rank r(500);
	r.load(NULL, 0);
	for(int i=1000000;i>0;--i)
	{
		if(i%375 == 0) r.reform();
		int t = rand();
		r.update(t%10000,t);
	}
	r.reform();
	r.dump(std::cout);
	return 0;
}

//callback
