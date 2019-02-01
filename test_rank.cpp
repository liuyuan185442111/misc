#include <sys/time.h>
#include "interval_ranking.h"

using namespace std;
using namespace grank;

int test_interval_ranking()
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
	return 0;
}

int main()
{
	test_interval_ranking();
	return 0;
}
