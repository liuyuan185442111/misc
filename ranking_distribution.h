#ifndef RANKING_DISTRIBUTION_H
#define RANKING_DISTRIBUTION_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <cstring>
#include <set>

namespace grank
{

template <typename ScoreType>
class IDistribution
{
protected:
	virtual int get_section(ScoreType score)=0;
public:
	virtual ~IDistribution(){}
	virtual bool init()=0;
	virtual bool add_data(ScoreType score)=0;
	virtual void add_end()=0;
	virtual int get_rank(ScoreType score)=0;
	virtual void change_score(ScoreType old_score, ScoreType new_score)=0;
	virtual void dump(std::ostream &out)=0;
};

template <typename ScoreType, typename SpaceType, size_t N>
class Distribution : public IDistribution<ScoreType>
{
protected:
	struct Bucket
	{
		SpaceType low_score;
		SpaceType high_score;
		size_t count;
		size_t higher_count;
	};
	ScoreType min_score;
	ScoreType max_score;
	SpaceType interval;
	Bucket buckets[N];

	int get_section(ScoreType score)
	{
		if(score < min_score || score >= max_score)
			return -1;
		return (score - min_score) / interval;
	}
	void change_section(int sec1, int sec2)
	{
		if(sec1 != sec2 && !(sec1 < 0 && sec2 < 0))
		{
			if(sec1 >= 0) --buckets[sec1].count;
			if(sec2 >= 0) ++buckets[sec2].count;
			for(int i=sec1<0?0:sec1; i<sec2; ++i)
			{
				++buckets[i].higher_count;
			}
			for(int i=sec2<0?0:sec2; i<sec1; ++i)
			{
				--buckets[i].higher_count;
			}
		}
	}

public:
	//把N作为构造函数的参数, 可以省掉init函数
	Distribution(ScoreType min, ScoreType max) : min_score(min), max_score(max), interval(0)
	{
		memset(buckets, 0, sizeof(Bucket) * N);
	}
	bool init()
	{
		if(N < 3) return false;
		interval = SpaceType(max_score - min_score) / N;
		if(interval < 0) return false;
		SpaceType t = min_score;
		for(size_t i=0; i<N; ++i)
		{
			buckets[i].low_score = t;
			t += interval;
			buckets[i].high_score = t;
		}
		return true;
	}
	bool add_data(ScoreType score)
	{
		int section = get_section(score);
		if(section < 0) return false;
		++buckets[section].count;
		return true;
	}
	void add_end()
	{
		for(size_t i=N, sum=0; i>0; --i)
		{
			buckets[i-1].higher_count = sum;
			sum += buckets[i-1].count;
		}
	}
	int get_rank(ScoreType score)
	{
		int section = get_section(score);
		if(section < 0) return section;
		Bucket &b = buckets[section];
		return b.higher_count + (int)((b.high_score - score) / interval * b.count);
	}
	void change_score(ScoreType old_score, ScoreType new_score)
	{
		change_section(get_section(old_score), get_section(new_score));
	}
	void add_score(ScoreType new_score)
	{
		change_section(-1, get_section(new_score));
	}
	void del_score(ScoreType old_score)
	{
		change_section(get_section(old_score), -1);
	}

	void dump(std::ostream &out = std::cout)
	{
		for(size_t i=0; i<N; ++i)
		{
			out << "bucket " << i << ": score=[" << buckets[i].low_score << "," << buckets[i].high_score << ")"
				<< ", cur_count=" << buckets[i].count << ", higher_count=" << buckets[i].higher_count << std::endl;
		}
	}
};

}
#endif

