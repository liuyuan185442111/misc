#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#define BM_ENABLE_GOOD_TABLE
#include "boyermoore.h"
#include "sunday.h"

using std::string;
using std::cout;
using std::endl;
unsigned char table[]=
{
	'0','1','2','3','4','5','6','7','8','9','<','>',
	'A','B','C','D','E','F','G','H','I','J','K','L','M',
	'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
	'a','b','c','d','e','f','g','h','i','j','k','l','m',
	'n','o','p','q','r','s','t','u','v','w','x','y','z'
};
class FindSubstr
{
	std::vector<string> strs;
public:
	void make_strs(int min_len, int max_len, int count)
	{
		srand(0);
		for(int i=0; i<count; ++i)
		{
			int len = rand() % (max_len - min_len) + min_len;
			string str(len, 0);
			for(int j=0; j<len; ++j)
				str[j] = table[rand() % sizeof(table)];
			strs.push_back(str);
		}
		/*
		for(int i=0; i<count; ++i)
			strs.push_back("11111111111111111111111111111111111111111111111111111112");
		*/
	}
	int search(const string &str)
	{
		Sunday bm(str.c_str());
		int ret_c = 0;
		for(const auto &i:strs)
		{
			if(strcmp(bm.findin(i.c_str()), "") != 0) ++ret_c;
		}
		return ret_c;
	}
};

#include <unistd.h>
#include <sys/time.h>
int main()
{
	FindSubstr s;
	s.make_strs(3200, 6400, 10000);
	struct timeval start, stop;
	gettimeofday(&start, NULL);
	//cout << "find " << s.search("1111111111111111111111112");
	cout << "find " << s.search("45645678901234567890456456789012345678904564567890123456789045645678904564567890123456789045645678901234567890456456789012345678904564567890");
	gettimeofday(&stop, NULL);
	cout << " using " << stop.tv_sec*1000 + stop.tv_usec/1000 - start.tv_sec*1000 - start.tv_usec/1000 << "ms" << endl;
	return 0;
}

