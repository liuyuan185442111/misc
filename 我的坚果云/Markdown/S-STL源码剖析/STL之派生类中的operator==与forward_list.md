list和slist的实现里，节点类型和迭代器类型都使用了分层结构。
源码剖析189页下方说明operator==()，类比它写出这个例子：
```cpp
#include <iostream>
using namespace std;

struct A
{
    A(int m):a(m) {}
    int a;
    bool operator==(const A& t) const
    {
        return (a == t.a);
    }
};

struct B: public A
{
    B(int m, int n):A(m),b(n) {}
    int b;
};

int main()
{
    B b1(2,4), b2(2,6);
    cout << (b1==b2);
    cout << b1.operator==(b2);

    return 0;
}
```
B没有定义operator==()，所以调用的是继承自A的operator==()，参数b2安全转换为A的对象。
对B的对象执行==，并不会比较B对象里的b，只会比较a。
某些情况需要这种比较吧。

但是，还是没看到list中迭代器分层的意义，派生类里根本就没有成员变量。
##forward_list
slist是一个单向链表，c++11里提供了forward_list（在&lt;forward_list>中），二者差不多。

forward_list提供了一个特殊节点（并不是指针），这个节点并不用来存储数据，只用来索引。
begin()返回的是这个节点的next，这样在begin()插入也很容易完成。

对forward_list执行insert和erase会特别耗时，因为二者都是在position之前进行操作，只能从头节点开始遍历到position。
所以forward_list提供了insert_after()和erase_after()来在position之后进行操作。
还提供了splice_after()来在position之后进行拼接操作。

为了配合这些after操作，forward_list提供了before_begin()来获得begin()之前的节点：
```cpp
// forward_list::before_begin
#include <iostream>
#include <forward_list>

int main ()
{
  std::forward_list<int> mylist = {20, 30, 40, 50};

  mylist.insert_after ( mylist.before_begin(), 11 );  // 11 20 30 40 50

  std::cout << "mylist contains:";
  for ( int& x: mylist ) std::cout << ' ' << x;
  std::cout << '\n';

  return 0;
}
```

其他内容请参考：http://www.cplusplus.com/reference/forward_list/forward_list/