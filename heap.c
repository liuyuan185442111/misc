#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
using namespace std;

void print_help()
{
    cout << "top [gen|set|heap1|heap2|heap3|test]" << endl;
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
        top.insert(t);
        if(top.size() > n)
            top.erase(top.begin());
    }
    fs.close();
    ostream_iterator<int> out_it(std::cout, ",");
    copy(top.rbegin(), top.rend(), out_it);
    cout << endl;
}

/** 使用STL提供的堆相关的函数获得top n, 这里依赖库的实现方式, 实际使用前需要进行测试
或者自行实现堆算法, 参考 https://blog.csdn.net/liuyuan185442111/article/details/45920979
pop_heap的gcc实现大概是先将first和last-1位置的值互换，这样first位置的值就是不对的了，
然后调用__adjust_heap将[first, last-1)调整为堆。
所以gcc的pop_heap实际上只需要[first,last-1)是一个合法的堆。*/
void get_top_heap1(int n)
{
    fstream fs("sample.txt", fstream::in);
    int top[n+2];
    for(int i=0; i<n+1; ++i)
    {
        if(!(fs >> top[i])) return;
    }
    make_heap(top, top+n+1, greater<int>());
    while(fs >> top[n+1])
    {
        pop_heap(top, top+n+2, greater<int>());
    }
    fs.close();
    pop_heap(top, top+n+1, greater<int>());
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
    int top[n+1];
    for(int i=0; i<n+1; ++i)
    {
        if(!(fs >> top[i])) return;
    }
    make_heap(top, top+n+1, greater<int>());
    for(int t; fs >> t;)
    {
        //取出堆的根节点, 将t作为新的根节点, 然后进行调整
        //上面直接用pop_heap的话实际上是将top[n+1]作为t的存放处
        __adjust_heap(top, 0, n+1, t, greater<int>());
    }
    fs.close();
    pop_heap(top, top+n+1, greater<int>());
    sort_heap(top, top+n, greater<int>());
    ostream_iterator<int> out_it(std::cout, ",");
    copy(top, top+n, out_it);
    cout << endl;
}

/** 将操作抽象成一个模板类, 并对样本数量小于maxsize的情况做了处理
*/
template <typename Key, int maxsize, typename Cmp = std::greater<Key> >
class TopTable
{
    int top[maxsize+1];
    int size;
public:
    TopTable():size(0){}
    void Push(const Key &key)
    {
        if(size < maxsize+1)
        {
            top[size++] = key;
            if(size == maxsize+1)
                make_heap(top, top+size, Cmp());
            return;
        }
        __adjust_heap(top, 0, maxsize+1, key, Cmp());
    }
    void Build(std::vector<Key> &table)
    {
        if(size == maxsize+1)
        {
            pop_heap(top, top+maxsize+1, Cmp());
            sort_heap(top, top+maxsize, Cmp());
            table.reserve(maxsize);
            table.assign(top, top+maxsize);
        }
        else
        {
            sort(top, top+size);
            table.reserve(size);
            table.assign(top, top+size);
        }
    }
};

void get_top500()
{
    TopTable<int, 500> table;
    fstream fs("sample.txt", fstream::in);
    for(int t; fs >> t; )
    {
        table.Push(t);
    }
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
        else if(strcmp(argv[1], "test") == 0)
        {
            clock_t t = clock();
            cout << t << endl;
        }
        else
            print_help();
        return 0;
    }
    print_help();
    return 0;
}