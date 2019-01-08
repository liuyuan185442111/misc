#include <cstddef>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>

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

	std::string _data;

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
	void sort()
	{
		std::sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
	}
	void save()
	{
		int *buf = (int *)_rank - 1;
		*buf = (int)_hashMap.size();
		sizeof(int) + sizeof(RankInfo) * *buf;
	}
	void load()
	{
	}

public:
	RankingList(size_t n) : _cmpor(ScoreCmp()), _maxsize(n?n:1), _min(nullptr)
	{
		_rank = (RankInfo *)((int *)malloc(sizeof(int) + n * sizeof(RankInfo)) + 1);
		_hashMap.reserve(_maxsize);
	}
	~RankingList() { free((int *)_rank - 1); }

	void update(KeyType key, ScoreType score, const InfoType &info = InfoType())
	{
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
			}
			else if(_cmpor(_min->score, score))
			{
				//substitute
				_hashMap.erase(_min->key);
				*_min = RankInfo(key, score, info);
				_hashMap.insert(std::make_pair(key, _min));
				update_min();
			}
		}
	}

	void reform()
	{
		sort();
		save();
		update_min_and_map();
	}

	void dump(std::ostream &out)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << ", last_ranking:" << i->last_ranking << std::endl;
	}

	void insert_sort(int *a, int n)
	{
		int picket;
		for (int i=1, j; i<n; ++i)
		{
			picket = a[i];
			for (j=i-1; j>=0&&a[j]>picket; --j)
				a[j+1] = a[j];
			a[j+1] = picket;
		}
	}
};


typedef RankingList<int, int, int> rank;
int main()
{
	srand(time(NULL));
	rank r(100);
	for(int i=1000000;i>0;--i)
	{
		if(i%100 == 0) r.reform();
		int t = rand();
		r.update(t%10000,t);
	}
	r.reform();
	r.dump(std::cout);
	return 0;
}

//todo
//callback
//full
//different sorts
