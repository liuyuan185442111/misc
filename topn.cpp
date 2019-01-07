#include <cstddef>
#include <functional>
#include <unordered_map>
template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp=std::less<ScoreType>>
class RankImp
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
	ScoreCmp cmpor;
	size_t maxsize;
	RankInfo *rank;
	RankInfo *min;
	std::unordered_map<int, RankInfo*> rankMap;
	void UpdateMin()
	{
		for(auto i = rank, e = rank + rankMap.size(); i !=  e; ++i)
			if(cmpor(i->score, min->score)) min = i;
	}

public:
	RankImp(size_t n) : cmpor(ScoreCmp()), maxsize(n?n:1), min(nullptr)
	{
		rank = (RankInfo *)malloc(n * sizeof(RankInfo));
		rankMap.reserve(maxsize);
	}
	~RankImp() { free(rank); }

	void Update(KeyType key, ScoreType score, const InfoType &info = InfoType())
	{
		auto it = rankMap.find(key);
		if(it != rankMap.end())
		{
			//found in rankMap
			it->second->score = score;
			it->second->other_info = info;
			if(it->second == min)
			{
				UpdateMin();
			}
		}
		else
		{
			size_t cursize = rankMap.size();
			if(cursize != maxsize)
			{
				//rankMap not full
				rankMap.insert(std::make_pair(key, &(rank[cursize] = RankInfo(key, score, info))));
				if(!min || cmpor(score, min->score))
				{
					min = rank + cursize - 1;
				}
			}
			else if(cmpor(min->score, score))
			{
				//substitute
				*min = RankInfo(key, score, info);
				UpdateMin();
			}
			//do nothing
		}
	}

	void Update()
	{
	}
};

#include <iostream>
typedef RankImp<int, int, int> rank;
int main()
{
	rank r(10);
	r.Update(1,1);
	r.Update(2,2);
	r.Update(3,3);
	r.Update(4,4);
	return 0;
}
