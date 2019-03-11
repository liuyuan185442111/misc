#include <iostream>
#include <string>
#include <vector>

//#define BM_ENABLE_GOOD_TABLE
#include "boyermoore.h"
#include "sunday.h"

using std::string;
using std::cout;
using std::endl;
unsigned char table[]=
{
	'0','1','2','3','4','5','6','7','8','9','<','>',' ',
	'A','B','C','D','E','F','G','H','I','J','K','L','M',
	'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
	'a','b','c','d','e','f','g','h','i','j','k','l','m',
	'n','o','p','q','r','s','t','u','v','w','x','y','z'
};
class FindStr
{
	std::vector<string> strs;
public:
	void make_strs(int min_len, int max_len, int count)
	{
		srand(99999);
		for(int i=0; i<count; ++i)
		{
			int len = rand() % (max_len - min_len) + min_len;
			string str(len, 0);
			for(int j=0; j<len; ++j)
				str[j] = table[rand() % sizeof(table)];
			strs.push_back(str);
		}
	}
	int search1(const string &str)
	{
		int ret_c = 0;
		for(const auto &i:strs)
		{
			if(i.find(str) != string::npos) ++ret_c;
		}
		return ret_c;
	}
	int search2(const string &str)
	{
		BoyerMoore bm(str.c_str());
		int ret_c = 0;
		for(const auto &i:strs)
		{
			if(bm.findin(i.c_str())[0]) ++ret_c;
		}
		return ret_c;
	}
	int search3(const string &str)
	{
		Sunday s(str.c_str());
		int ret_c = 0;
		for(const auto &i:strs)
		{
			if(s.findin(i.c_str())[0]) ++ret_c;
		}
		return ret_c;
	}
};

#include <unistd.h>
#include <sys/time.h>
int main()
{
	FindStr s;
	s.make_strs(5000, 9800, 10000);

	string pattern("p4rDtntwBOohZzovBXPIvmQyAlW3BsV1aY3tUofJRt9T3es1aj4UKG0Da04m6F0Qkyem6KzoPucNMU7gyPoXDpbKCYyiqXZSIJkHhOi3719vNCsOyJdofOYinSJxhWNZjVvuBME92OfYCqxKPjtLiNd1U2Aeqfpuq2LWaaw9iIFUEnMVesydIX4Q1Lj5uJKq91JPHht4z5EZ4OidE3RhdcIoyVFsbhwXDSeCQVFilps5oaVY7SfHEwnQcWffR7eT4dqV1cvSJ6jqzaWA1jGlpirfIvSzyn9v6ZNP7nyagxzs");
	pattern = "890";
	struct timeval start, stop;
	gettimeofday(&start, nullptr);
	cout << "find " << s.search1(pattern.c_str()) << endl;
	gettimeofday(&stop, nullptr);
	cout << "search1 using " << stop.tv_sec*1000 + stop.tv_usec/1000 - start.tv_sec*1000 - start.tv_usec/1000 << "ms" << endl;
	gettimeofday(&start, nullptr);
	cout << "find " << s.search2(pattern.c_str()) << endl;
	gettimeofday(&stop, nullptr);
	cout << "search2 using " << stop.tv_sec*1000 + stop.tv_usec/1000 - start.tv_sec*1000 - start.tv_usec/1000 << "ms" << endl;
	gettimeofday(&start, nullptr);
	cout << "find " << s.search3(pattern.c_str()) << endl;
	gettimeofday(&stop, nullptr);
	cout << "search3 using " << stop.tv_sec*1000 + stop.tv_usec/1000 - start.tv_sec*1000 - start.tv_usec/1000 << "ms" << endl;

	return 0;
}

