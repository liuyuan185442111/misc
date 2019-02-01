#include <sys/time.h>
#include "interval_ranking.h"
#include "realtime_ranking.h"
#include "ranking_distribution.h"

using namespace std;
using namespace grank;

void test_interval_ranking()
{
	srand(time(nullptr));
	srand(0);
	IntervalRankingList<int, int> r(100);
	r.load(nullptr, 0);
	for(int i=1000000;i>0;--i)
	{
		if(i%100 == 0) r.reform();
		int t = rand();
		r.update(t%10000,t);
		if(rand()&1) r.remove(t%10000);
	}
	r.reform();
	r.dump(std::cout);
	std::cout << std::endl;

	srand(0);
	IntervalRankingList<int, int> s(100);
	s.load(nullptr, 0);
	for(int i=10000;i>0;--i)
	{
		int t = rand();
		s.update(t%10000,t);
	}
	s.reform();
	s.dump();
	std::cout << std::endl;
}

void test_realtime_ranking()
{
	srand(time(nullptr));
	RealtimeRankingList<int, int, std::string> r(40);
	for(int i=1000000; i>0; --i)
	{
		int t = rand();
		r.update(t%10000, t);
		if(rand()&1) r.remove(t%10000);
	}
	r.dump(std::cout);
}

void test_realtime_ranking2()
{
	RealtimeRankingList<int, int, int> r(1);
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
}

typedef IDistribution<int> CommonDist;
typedef Distribution<int, float, 100> CommonDist100;
void test_ranking_distribution()
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
}

int main()
{
	test_interval_ranking();
	test_realtime_ranking();
	test_realtime_ranking2();
	test_ranking_distribution();
	return 0;
}
