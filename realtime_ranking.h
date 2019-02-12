#ifndef REALTIME_RANKING_H
#define REALTIME_RANKING_H

#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>

namespace grank
{

template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp = std::greater<ScoreType>>
class RealtimeRankingList
{
public:
	struct RankInfo
	{
		KeyType key;
		ScoreType score;
		InfoType info;
		RankInfo(KeyType k, ScoreType s, InfoType i) : key(k), score(s), info(i) {}
	};

private:
	ScoreCmp _cmpor;
	size_t _maxsize;
	RankInfo *_rank;
	std::unordered_map<KeyType, RankInfo*> _hashMap;

public:
	RealtimeRankingList(size_t n) : _cmpor(ScoreCmp()), _maxsize(n?n:1)
	{
		_rank = (RankInfo *)(malloc(_maxsize * sizeof(RankInfo)));
		for(RankInfo *p = _rank, *pe = _rank + _maxsize; p != pe; ++p)
			new(p) RankInfo(KeyType(), ScoreType(), InfoType());
		_hashMap.reserve(_maxsize);
	}
	~RealtimeRankingList()
	{
		for(RankInfo *p = _rank, *pe = _rank + _maxsize; p != pe; ++p)
			p->~RankInfo();
		free(_rank);
	}

	bool update(KeyType key, ScoreType score, const InfoType &info = InfoType());
	bool remove(KeyType key);
	void dump(std::ostream &out = std::cout)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << std::endl;
	}
	const RankInfo *begin() { return _rank; }
	const RankInfo *end() { return _rank + size(); }
	size_t size() { return _hashMap.size(); }
};



template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp>
bool RealtimeRankingList<KeyType, ScoreType, InfoType, ScoreCmp>::update(KeyType key, ScoreType score, const InfoType &info)
{
	auto it = _hashMap.find(key);
	if(it != _hashMap.end())
	{
		//found in _rank
		RankInfo *target = it->second;
		if(_cmpor(score, target->score))
		{
			if(target != _rank)
			{
				RankInfo *p = target;
				for( ; p != _rank; --p)
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
				target->score = score;
				target->info = info;
			}
		}
		else if(_cmpor(target->score, score))
		{
			if(target != _rank + _hashMap.size() - 1)
			{
				RankInfo *p = target;
				for(RankInfo *pe = _rank + _hashMap.size() - 1; p != pe; ++p)
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
				target->score = score;
				target->info = info;
			}
		}
	}
	else
	{
		size_t cursize = _hashMap.size();
		if(cursize != _maxsize)
		{
			RankInfo *p = _rank;
			if(cursize)
			{
				//_rank not full
				RankInfo *pos = std::upper_bound(_rank, _rank + cursize, RankInfo(key,score,info),
						[this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(lhs.score, rhs.score);});
				for(p += cursize; p != pos; --p)
				{
					*p = *(p-1);
					_hashMap[p->key] = p;
				}
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
			for( ; p != _rank; --p)
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
			_hashMap.insert(std::make_pair(key, p));
		}
		else
		{
			//do nothing
			return false;
		}
	}
	return true;
}

template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp>
bool RealtimeRankingList<KeyType, ScoreType, InfoType, ScoreCmp>::remove(KeyType key)
{
	auto it = _hashMap.find(key);
	if(it != _hashMap.end())
	{
		auto del_key = it->first;
		for(auto p(it->second), pe(_rank+_hashMap.size()-1); p != pe; ++p)
		{
			*p = *(p+1);
			_hashMap[p->key] = p;
		}
		_hashMap.erase(del_key);
		return true;
	}
	return false;
}

}
#endif

