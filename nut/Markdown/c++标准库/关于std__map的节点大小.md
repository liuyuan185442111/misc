项目中需要用到很大的map，于是想看一下map本身的存储消耗是多少，于是写了个最简单的allocator来试。
[以下MyAlloc的默认构造、（广义的）复制构造、rebind是必需的，为了查看分配空间的大小，allocate也是必需的。][allocator]
用greater&lt;int>()显式的构造map是为了说明，[“c++尽可能将语句解释为函数声明”][Effective]，语句

	map<int,int,greater<int>,MyAlloc<pair<const int,int> > > m(greater<int>());
将被编译器解释为一个函数声明，形式参数是一个省略名字的函数指针。
把形式参数的声明用括号括起来是非法的，给函数参数加上括号却是合法的，所以给greater&lt;int>()套上括号就可以了。
以下是完整代码：
```
#include <iostream>
#include <map>
using namespace std;

//#pragma pack(1)

template <typename T>
class MyAlloc : public allocator<T>
{
public:
    typename allocator<T>::pointer allocate(typename allocator<T>::size_type n, const void *p=0)
    {
        cout << "allocate " << n << " element whose size is " << sizeof(T) << endl;
        return allocator<T>::allocate(n,p);
    }
    //用于构造MyAlloc
    MyAlloc(){}
    //使MyAlloc可用于红黑树节点
    template <typename U> struct rebind
    {
        typedef MyAlloc<U> other;
    };
    //用于容器的get_allocator方法
    template <typename U> MyAlloc(const MyAlloc<U>& alloc){}
};

int main()
{
    map<int,int,greater<int>,MyAlloc<pair<const int,int> > > m((greater<int>()));
    m[1] = 101;
    m[2] = 102;
    m[3] = 103;
    cout << "first key is " << m.begin()->first << endl;
    return 0;
}
```
gcc4.1.2中每个节点大小是24，在4.3.2,4.4.4,6.3中每个节点的大小是40。
可见如果key和value只是int，map本身的存储开销还是很大的。
[allocator]: https://blog.csdn.net/liuyuan185442111/article/details/45743345 "allocator"
[Effective]: https://blog.csdn.net/liuyuan185442111/article/details/76767509#t3 "Effective STL"