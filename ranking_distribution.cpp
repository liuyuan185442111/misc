#include <functional>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <cstring>
#include <set>

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
		int sec1 = get_section(old_score);
		int sec2 = get_section(new_score);
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
	multiset<int> scores;
	CommonDist *p = new CommonDist100(1, 8001);
	p->init();
	for(int i=0; i<9999; ++i)
	{
		int score = rand() % 7999 + 1;
		p->add_data(score);
		scores.insert(score);
	}
	p->add_end();

	cout << "rank distribution:\n";
	p->dump(cout);

	{
		auto a = scores.begin();
		auto b = scores.rbegin();
		for(int i=scores.size()/2-100; i>0; --i)
		{
			p->change_score(*a, *b);
			p->change_score(*b, *a);
		}
	}
	cout << "rank distribution after change score:\n";
	p->dump(cout);

	cout << "score:real_rank:approximate_rank\n";
	int c = 0;
	for(auto i=scores.rbegin(), e=scores.rend(); i!=e; ++i)
	{
		cout << *i << ":" << ++c << ":" << p->get_rank(*i) << endl;
	}

	delete p;
	return 0;
}
