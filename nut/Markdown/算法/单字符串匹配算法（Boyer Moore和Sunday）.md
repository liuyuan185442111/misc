字符串匹配（查找）算法是一类重要的字符串算法，给定一个长度为n的字符串text，要求找出text中是否存在另一个长度为m的字符串pattern（模式串）。
主要方法有朴素算法，KMP算法，BM（Boyer Moore）算法，Sunday算法。
朴素算法就是暴力匹配，KMP非常有名，但效率感人，不多说。
关于[Boyer-Moore算法][bm]和[Sunday算法][sunday]，[《字符串匹配的Boyer-Moore算法》][bm]和[《字符串匹配算法之Sunday算法》][sunday]两篇文章讲得很清楚。
[《字符串匹配算法之Sunday算法》](https://www.jianshu.com/p/2e6eb7386cd3)对他们的效率进行了分析。

[bm]: http://www.ruanyifeng.com/blog/2013/05/boyer-moore_string_search_algorithm.html
[sunday]: https://www.jianshu.com/p/2e6eb7386cd3

我实现了BM和Sunday算法，并同string::find做了对比，O3优化，测试发现pattern字符串比较小的时候，string::find优势巨大，比BM和Sunday高效数倍，pattern增大的过程中，BM渐渐和string::find相当了（但效率还是稍低），Sunday变得优秀了，花费的时间是string::find的37%左右。（注：1. Sunday始终比BM效率高 2.启不启用“好后缀”对BM影响不大）

为什么和理论分析不太一样呢？我猜测原因可能有：string::find可能并不是暴力匹配；标准库有优化；我的实现不够好；测试数据有问题；pattern字符串小的时候，BM和Sunday额外的操作带来的效率提升不如花费的成本。

结论：大部分时候string::find足够了，如有需要可以用Sunday算法。
BM算法比Sunday算法复杂的多，实现BM用了半天，实现Sunday只用了不到一个小时。简单可能更好。

附上测试代码：
```cpp
#include <iostream>
#include <string>
#include <vector>

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

find 0
search1 using 76ms
find 0
search2 using 82ms
find 0
search3 using 29ms
```

