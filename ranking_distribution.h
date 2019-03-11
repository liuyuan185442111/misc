#ifndef RANKING_DISTRIBUTION_H
#define RANKING_DISTRIBUTION_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <cstring>
#include <vector>
#include <set>
#include <assert.h>

namespace grank
{

template <typename ScoreType>
class IDistribution
{
protected:
	virtual int get_section(ScoreType score)=0;
public:
	virtual ~IDistribution(){}
	virtual bool add_data(ScoreType score)=0;
	virtual void add_end()=0;
	virtual int get_rank(ScoreType score)=0;
	virtual void change_score(ScoreType old_score, ScoreType new_score)=0;
	virtual void add_score(ScoreType new_score)=0;
	virtual void del_score(ScoreType old_score)=0;
	virtual void dump(std::ostream &out)=0;
	virtual void load(const std::vector<int> &counts)=0;
	virtual void save(std::vector<int> &counts)=0;
};

template <typename ScoreType, typename SpaceType>
class Distribution : public IDistribution<ScoreType>
{
protected:
	struct Bucket
	{
		SpaceType low_score;
		SpaceType high_score;
		size_t count;
		size_t higher_count;
		Bucket(SpaceType low, SpaceType high)
			: low_score(low), high_score(high), count(0), higher_count(0) {}
	};
	ScoreType min_score;
	ScoreType max_score;
	SpaceType interval;
	std::vector<Bucket> buckets;

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
			for(int i=(sec1<0?0:sec1); i<sec2; ++i)
			{
				++buckets[i].higher_count;
			}
			for(int i=(sec2<0?0:sec2); i<sec1; ++i)
			{
				--buckets[i].higher_count;
			}
		}
	}

public:
	//[min,max)
	Distribution(ScoreType min, ScoreType max, size_t pieces = 100) : min_score(min), max_score(max)
	{
		if(max_score < min_score) std::swap(min_score, max_score);
		assert(min_score < max_score);
		assert(pieces > 3);
		interval = SpaceType(max_score - min_score) / pieces;
		assert(interval > 0);

		buckets.reserve(pieces);
		SpaceType t = min_score;
		for(size_t i=0; i<pieces; ++i)
		{
			buckets.push_back(Bucket(t, t + interval));
			t += interval;
		}
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
		for(size_t i=buckets.size(), sum=0; i>0; --i)
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
		for(size_t i=0; i<buckets.size(); ++i)
		{
			out << "bucket " << i << ": score=[" << buckets[i].low_score << "," << buckets[i].high_score << ")"
				<< ", cur_count=" << buckets[i].count << ", higher_count=" << buckets[i].higher_count << std::endl;
		}
	}

	void load(const std::vector<int> &counts)
	{
		if(counts.size() != buckets.size()) return;
		for(int i=0, s=(int)counts.size(); i!=s; ++i)
		{
			buckets[i].count = counts[i];
		}
		add_end();
	}
	void save(std::vector<int> &counts)
	{
		counts.clear();
		counts.reserve(buckets.size());
		for(auto &e:buckets)
		{
			counts.push_back(e.count);
		}
	}
};

}
#endif

