#include <cstddef>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <assert.h>
#include <map>
#include <unistd.h>
#include <sys/time.h>

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
		size_t higher_count;
		size_t count;
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
public:
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
		if(score < min_score || score >= max_score)
			return false;
		++buckets[get_section(score)].count;
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
		return b.higher_count + (int)((score - b.low_score) / (b.high_score - b.low_score) * b.count);
	}
	void change_score(ScoreType old_score, ScoreType new_score)
	{
		int sec1 = get_section(old_score);
		int sec2 = get_section(new_score);
		std::cout << sec1 << sec2 << std::endl;
		if(sec1 != sec2 && sec1 >= 0 && sec2 >= 0)
		{
			--buckets[sec1].count;
			++buckets[sec2].count;
			for(int i=sec1; i<sec2; ++i)
			{
				++buckets[i].higher_count;
			}
			for(int i=sec2; i<sec1; ++i)
			{
				--buckets[i].higher_count;
			}
		}
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

typedef IDistribution<int> CommonDist;
typedef Distribution<int, float, 100> CommonDist100;

using namespace std;

int main()
{
	CommonDist *p = new CommonDist100(1, 101);
	p->init();
	p->add_data(10);
	p->add_data(20);
	p->add_data(30);
	p->add_end();
	p->change_score(30, 100);
	p->dump(cout);
	cout << p->get_rank(9) << endl;
	return 0;
}
