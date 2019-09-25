历时一月左右，基本看完了c++标准库的源码，忽略了一些用处不那么大的部分，和一些复杂难以理解的算法：输入输出库，一些复杂的算法（stable_sort，stable_partition，inplace_merge，list的排序，红黑树的调整等）。

总体而言，收益还是蛮多的，首先了解了STL容器的用法，和内部实现，加深了对模板和类型推断的了解。学到了一些技巧，比如统计bitset中1个数使用的查表法，deque使用的双层存储结构，allocator使用的两级配置器，vector和string的内存增长策略，string的copy-on-write技术，partition使用的算法，函数对象的配接技术等等。

trait技法
使用模板特化技术，不同的特化版本中将不同类型typedef成相同名字，针对每种类型定义不同的函数模板来处理，我模仿的简单例子：
```cpp
#include <iostream>
using namespace std;

struct true_type{};
struct false_type{};

template <typename T>
struct traits
{
    typedef false_type type;
};

template <>
struct traits<int>
{
    typedef true_type type;
};

void process_aux(true_type)
{
    cout << "true_type\n";
}

void process_aux(false_type)
{
    cout << "false_type\n";
}

template <typename T>
void process(T m)
{
    typedef typename traits<T>::type type;
    process_aux(type());
}

int main()
{
    process(2.3);
    process(6);
    return 0;
}
```
这很大程度上得益于函数模板的类型推断机制，process(2.3)和process(6)仅参数类型不同，内部就有完全不同的实现。