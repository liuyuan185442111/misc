#include <cstddef>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <assert.h>
#include <map>

template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp=std::greater<ScoreType>>
class RankingList
{
	struct RankInfo
	{
		KeyType key;
		ScoreType score;
		InfoType info;
		RankInfo(KeyType k, ScoreType s, InfoType i) : key(k), score(s), info(i) {}
	};
	ScoreCmp _cmpor;
	size_t _wanted;
	size_t _maxsize;
	RankInfo *_rank;
	std::unordered_map<KeyType, RankInfo*> _hashMap;

public:
	RankingList(size_t n) : _cmpor(ScoreCmp()), _wanted(n?n:1), _maxsize(_wanted + std::max(_wanted/10, (size_t)10))
	{
		_rank = (RankInfo *)((int *)malloc(sizeof(int) + _maxsize * sizeof(RankInfo)) + 1);
		memset(_rank, 0, _maxsize * sizeof(RankInfo));
		_hashMap.reserve(_maxsize);
	}
	~RankingList() { free((int *)_rank - 1); }

	bool update(KeyType key, ScoreType score, const InfoType &info = InfoType())
	{
		auto it = _hashMap.find(key);
		if(it != _hashMap.end())
		{
			//found in _rank
			if(_cmpor(score, it->second->score))
			{
				if(it->second != _rank)
				{
					RankInfo *p = it->second;
					for(; p != _rank; --p)
					{
						if(_cmpor(score, (p-1)->score))
						{
							*p = *(p-1);
							_hashMap[p->key] = p;
						}
						else
							break;
					}
					p->key = key;
					p->score = score;
					p->info = info;
					_hashMap[key] = p;
				}
				else
				{
					it->second->score = score;
					it->second->info = info;
				}
			}
			else if(_cmpor(it->second->score, score))
			{
				if(it->second != _rank + _hashMap.size() - 1)
				{
					RankInfo *p = it->second;
					for(; p!=_rank+_hashMap.size()-1; ++p)
					{
						if(_cmpor((p+1)->score, score))
						{
							*p = *(p+1);
							_hashMap[p->key] = p;
						}
						else
							break;
					}
					p->key = key;
					p->score = score;
					p->info = info;
					_hashMap[key] = p;
				}
				else
				{
					it->second->score = score;
					it->second->info = info;
				}
			}
		}
		else
		{
			size_t cursize = _hashMap.size();
			if(cursize != _maxsize)
			{
				//_rank not full
				RankInfo *pos = std::upper_bound(_rank, _rank + cursize, RankInfo(key,score,info), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(lhs.score, rhs.score);});
				RankInfo *p = _rank + cursize;
				for(; p != pos; --p)
				{
					*p = *(p-1);
					_hashMap[(p-1)->key] = p;
				}
				p->key = key;
				p->score = score;
				p->info = info;
				_hashMap.insert(std::make_pair(key, p));
			}
			else if(_cmpor(score, (_rank + _maxsize - 1)->score))
			{
				RankInfo *p = _rank + _maxsize - 1;
				_hashMap.erase(p->key);
				for(; p!=_rank; --p)
				{
					if(_cmpor(score, (p-1)->score))
					{
						*p = *(p-1);
						_hashMap[p->key] = p;
					}
					else
						break;
				}
				p->key = key;
				p->score = score;
				p->info = info;
				_hashMap[key] = p;
			}
		}
	}
	bool remove(KeyType key)
	{
		auto it = _hashMap.find(key);
		if(it != _hashMap.end())
		{
			auto tk = it->first;
			for(RankInfo *p = it->second; p != _rank+_hashMap.size()-1; ++p)
			{
				*p = *(p+1);
				_hashMap[p->key] = p;
			}
			_hashMap.erase(tk);
		}
	}

	void dump(std::ostream &out = std::cout)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << std::endl;
	}
};

using std::string;
using std::cout;
using std::endl;

int topn()
{
	srand(time(NULL));
	srand(0);
	RankingList<int, int, int> r(100);
	for(int i=1000000;i>0;--i)
	{
		int t = rand();
		r.update(t%10000,t);
		if(rand()&1) r.remove(t%10000);
	}
	r.dump(std::cout);
	std::cout << std::endl;
	return 0;
}

#include <unistd.h>
#include <sys/time.h>
int main()
{
	topn();
	return 0;
	RankingList<int, int, int> r(1);
	r.update(1,2);
	r.update(2,2);
	r.update(3,4);
	r.update(4,4);
	r.update(5,6);
	r.update(6,6);
	r.dump(std::cout);
	cout << endl;
	r.update(5,2);
	r.dump(std::cout);
	return 0;
}

