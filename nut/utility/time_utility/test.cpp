#include <iostream>
using namespace std;
#include "time_utility.h"

void print_time(time_t time)
{
	struct tm *t = localtime(&time);
	cout << "time=" << time << ' ' << t->tm_year+1900 << '-' << t->tm_mon+1 << '-' << t->tm_mday;
	cout << ' ' << t->tm_hour << ':' << t->tm_min << ':' << t->tm_sec << ' ' << t->tm_wday << endl;
}
void test1(int cur_time)
{
	bool last = true;
	for(time_t t=cur_time-30*24*3600;t<cur_time+30*24*3600;++t)
	{
		if(last != is_another_day(cur_time, t, 8*3600))
		{
			last = is_another_day(cur_time, t, 8*3600);
			cout << "same_day ";
			print_time(t);
		}
	}
}
void test2(int cur_time, int start)
{
	bool last = true;
	for(time_t t=cur_time-30*24*3600;t<cur_time+30*24*3600;++t)
	{
		if(last != is_another_week_start(cur_time, t, 8*3600, start))
		{
			last = is_another_week_start(cur_time, t, 8*3600, start);
			cout << "same_week ";
			print_time(t);
		}
	}
}

int main()
{
	int t = 1234022400;
	cout << "cur_time ";
	print_time(t);
	test1(t);
	for(int i=0;i<=7;++i) test2(t,i);
	return 0;
}
