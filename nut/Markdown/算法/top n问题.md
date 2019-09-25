最近看到游戏中刷排行榜的问题，排行榜是Top N榜，每天定时离线刷榜。
现在是用multiset实现的，想到以前面试的时候这种问题的答案都是用堆，于是自己实现了一下，完整代码如下：
```cpp
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
using namespace std;

void print_help(const char *name)
{
    cout << name << " [gen|set|heap1|heap2|heap3]" << endl;
}

//产生count个随机数
void gen(const char *dest, int count)
{
    srand(time(NULL));
    std::fstream fs(dest, std::fstream::trunc | std::fstream::out);
    for(int i=0; i<count; ++i)
    {
        fs << rand() << endl;
    }
    fs.close();
}

//使用multiset来获得top n
void get_top_set(int n)
{
    multiset<int> top;
    fstream fs("sample.txt", fstream::in);
    for(int t; fs >> t;)
    {
		if(t > *top.begin())
		{
			top.insert(t);
			if(top.size() > n)
				top.erase(top.begin());
		}
    }
    fs.close();
    ostream_iterator<int> out_it(std::cout, ",");
    copy(top.rbegin(), top.rend(), out_it);
    cout << endl;
}

/** 使用STL提供的堆相关的函数获得top n, 这里依赖库的实现方式, 实际使用前需要进行确认,
或自行实现堆算法, 参考 https://blog.csdn.net/liuyuan185442111/article/details/45920979
pop_heap的gcc实现大概是先将first和last-1位置的值互换，这样first位置的值就不一定是对的了，
然后调用__adjust_heap将[first, last-1)调整为堆。
所以gcc的pop_heap实际上只要求[first,last-1)是一个合法的堆。*/
void get_top_heap1(int n)
{
    fstream fs("sample.txt", fstream::in);
    int top[n+1];
    for(int i=0; i<n; ++i)
    {
        if(!(fs >> top[i])) return;
    }
    make_heap(top, top+n, greater<int>());
	int *top_end = top+n+1;
    while(fs >> top[n])
    {
		if(top[n] > top[0])
			pop_heap(top, top_end, greater<int>());
    }
    fs.close();
    sort_heap(top, top+n, greater<int>());
    ostream_iterator<int> out_it(std::cout, ",");
    copy(top, top+n, out_it);
    cout << endl;
}

/** 与以上相比, 使用gcc内部的__adjust_heap函数
*/
void get_top_heap2(int n)
{
    fstream fs("sample.txt", fstream::in);
    int top[n];
    for(int i=0; i<n; ++i)
    {
        if(!(fs >> top[i])) return;
    }
    make_heap(top, top+n, greater<int>());
    for(int t; fs >> t;)
    {
        //取出堆的根节点, 将t作为新的根节点, 然后进行调整
        //上面直接用pop_heap的话实际上是将top[n+1]作为t的存放处
		if(t > top[0])
			__adjust_heap(top, 0, n, t, greater<int>());
    }
    fs.close();
    sort_heap(top, top+n, greater<int>());
    ostream_iterator<int> out_it(std::cout, ",");
    copy(top, top+n, out_it);
    cout << endl;
}

/** 将操作抽象成一个模板类, 并对样本数量小于maxsize的情况做了处理
*/
template <typename T, typename Cmp = std::greater<T> >
class TopTable
{
	std::vector<T> top;
	size_t maxsize;
	Cmp comparator;
public:
    TopTable(size_t n) : maxsize(n?n:0) { top.reserve(maxsize); }
    bool Push(const T &key)
    {
        if(top.size() < maxsize)
        {
			top.push_back(key);
            if(top.size() == maxsize)
                make_heap(top.begin(), top.end(), comparator);
            return true;
        }
		if(comparator(key, top[0]))
		{
			__adjust_heap(top.begin(), 0, (int)maxsize, key, comparator);
			return true;
		}
		return false;
    }
	template <typename U>
    size_t Build(std::vector<U> &table)
    {
		size_t cursize = top.size();
        if(cursize < maxsize)
            sort(top.begin(), top.end(), comparator);
        else
            sort_heap(top.begin(), top.end(), comparator);
		table.reserve(cursize);
		table.assign(top.begin(), top.end());
		return cursize;
    }
};

void get_top500()
{
    TopTable<int> table(500);
    fstream fs("sample.txt", fstream::in);
    for(int t; fs >> t; )
        table.Push(t);
    fs.close();
    std::vector<int> top;
    table.Build(top);
    ostream_iterator<int> out_it(std::cout, ",");
    copy(top.begin(), top.end(), out_it);
    cout << endl;
}

int main(int argc, char **argv)
{
    if(argc == 2)
    {
        if(strcmp(argv[1], "gen") == 0)
            gen("sample.txt", 10000000);
        else if(strcmp(argv[1], "set") == 0)
            get_top_set(500);
        else if(strcmp(argv[1], "heap1") == 0)
            get_top_heap1(500);
        else if(strcmp(argv[1], "heap2") == 0)
            get_top_heap2(500);
        else if(strcmp(argv[1], "heap3") == 0)
            get_top500();
        else
            print_help(argv[0]);
        return 0;
    }
    print_help(argv[0]);
    return 0;
}
```
另外写了个函数测试两种方式的效率：
```cpp
void test(int n)
{
    fstream fs("sample.txt", fstream::in);
	std::vector<int> sample;
    for(int t; fs >> t;)
		sample.push_back(t);
	fs.close();

	{
		struct timeval start_time, stop_time;
		gettimeofday(&start_time, NULL);
		multiset<int> top;
		for(std::vector<int>::iterator it=sample.begin(), ie=sample.end(); it!=ie; ++it)
		{
			if(*it > *top.begin())
			{
				top.insert(*it);
				if(top.size() > n)
					top.erase(top.begin());
			}
		}
		gettimeofday(&stop_time, NULL);
		cout << "set use " << (stop_time.tv_sec-start_time.tv_sec)*1000+(stop_time.tv_usec-start_time.tv_usec)/1000 << " ms" << endl;
	}

	{
		struct timeval start_time, stop_time;
		gettimeofday(&start_time, NULL);
		int top[n];
		std::vector<int>::iterator it=sample.begin();
		for(int i=0; i<n; ++i, ++it)
		{
			top[i] = *it;
		}
		make_heap(top, top+n, greater<int>());
		for(std::vector<int>::iterator ie=sample.end(); it!=ie; ++it)
		{
			if(*it > top[0])
				__adjust_heap(top, 0, n, *it, greater<int>());
		}
		sort_heap(top, top+n, greater<int>());
		gettimeofday(&stop_time, NULL);
		cout << "heap use " << (stop_time.tv_sec-start_time.tv_sec)*1000+(stop_time.tv_usec-start_time.tv_usec)/1000 << " ms" << endl;
	}
}
```
test(500)的`for i in {1..10}; do ./a.out; done`输出结果如下：
set use 201 ms
heap use 92 ms
set use 200 ms
heap use 91 ms
set use 203 ms
heap use 92 ms
set use 204 ms
heap use 93 ms
set use 205 ms
heap use 92 ms
set use 203 ms
heap use 95 ms
set use 202 ms
heap use 92 ms
set use 208 ms
heap use 92 ms
set use 201 ms
heap use 91 ms
set use 203 ms
heap use 92 ms
可以看到提升还是很大的，不过算上从文件中读取数据的时间，或遍历数据库的时间，这点提升就无所谓了。

参考：[STL之heap](https://blog.csdn.net/liuyuan185442111/article/details/45920979)

---
实时的榜则比较复杂，最近在研究，后续完善
[关于实时TopN排名算法的思考](https://blog.csdn.net/vipally/article/details/84329809)
[使用redis实现实时排行榜](https://blog.csdn.net/weixin_37799190/article/details/81982042)

