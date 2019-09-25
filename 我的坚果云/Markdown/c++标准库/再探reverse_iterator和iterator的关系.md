reference：Effective STL:item 28 了解如何通过reverse_iterator的base得到iterator
```
#include <iterator>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
using namespace std;

/**
 begin         i               end
|  1  |  2  |  3  |  4  |  5  |
  rend               ri        rbegin

reverse_iterator内部拥有一个iterator,是一个适配器,主要特殊之处在于:
    reference operator*() const
    {
        Iterator tmp = current;
        return *--tmp;
    }
所以对rbegin()内部存储的是end(),但解引用的值是5.*ri的值是3,所以find(v.rbegin(), v.rend(), 3);的返回值就是ri.
参考我的博客《STL之迭代器》
*/

int main()
{
    int t[5] = {1,2,3,4,5};
    vector<int> v(t, t+5);
    vector<int>::iterator i = find(v.begin(), v.end(), 3);
    vector<int>::reverse_iterator ri = find(v.rbegin(), v.rend(), 3);
    assert(*i == 3);
    assert(*ri == 3);
    assert(ri.base()-i == 1);
    //若要删除元素3
    v.erase(--ri.base());//或者使用 v.erase((++ri).base());
    ostream_iterator<int> out_it(cout,", ");
    copy(v.begin(), v.end(), out_it);
}

/**
但《Effective STL》指出,对于一些vector和string的一些实现,"v.erase(--ri.base());"无法通过编译.
因为在这样的实现中,iterator是以内置指针的方式来实现的,所以ri.base()的结果是一个指针,而自加操作符不能用于rvalue,例如:
int getint()
{
    return 6;
}
int main()
{
    ++getint();//error
    return 0;
}
如果ri.base()是一个真正的对象就没问题了,自加操作会变成调用操作符函数.
或者,将v.erase(--ri.base());替换为v.erase(ri.base()-1);就没有问题了.
*/
```