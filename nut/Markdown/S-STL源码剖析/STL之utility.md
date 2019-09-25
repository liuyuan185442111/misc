utility里包含两样东西：通用关系比较操作符（Generic relational operators）和pair。
通用关系比较操作符有6种，分别是==, !=, <, >, <=, >=。
pair是可以容纳两个类型的容器。
##generic relational operators
比较操作符定义于stl_relops.h中：
```cpp
namespace std
{
namespace rel_ops
{

template <class _Tp>
inline bool operator!=(const _Tp& __x, const _Tp& __y)
{
    return !(__x == __y);
}

template <class _Tp>
inline bool operator>(const _Tp& __x, const _Tp& __y)
{
    return __y < __x;
}

template <class _Tp>
inline bool operator<=(const _Tp& __x, const _Tp& __y)
{
    return !(__y < __x);
}

template <class _Tp>
inline bool operator>=(const _Tp& __x, const _Tp& __y)
{
    return !(__x < __y);
}

} // namespace rel_ops
} // namespace std
```
我并没有把std拿掉，因为这里面定义了一个rel_ops命名空间，为了整体可读性就没取掉std。
这四个操作符函数，使得针对某个类已经定义过==和<之后，不必再定义其他四个比较运算符，只要包含utility这个头文件即可。
看下面这个例子：
```cpp
#include <iostream>     // std::cout, std::boolalpha
#include <utility>      // std::rel_ops
#include <cmath>        // std::sqrt

class vector2d {
public:
  double x,y;
  vector2d (double px,double py): x(px), y(py) {}
  double length() const {return std::sqrt(x*x+y*y);}
  bool operator==(const vector2d& rhs) const {return length()==rhs.length();}
  bool operator< (const vector2d& rhs) const {return length()< rhs.length();}
};

int main () {
  using namespace std::rel_ops;
  vector2d a (10,10);	// length=14.14
  vector2d b (15,5);	// length=15.81
  std::cout << std::boolalpha;
  std::cout << "(a<b) is " << (a<b) << '\n';
  std::cout << "(a>b) is " << (a>b) << '\n';
  return 0;
}
```
sgi对这个头文件有这样的注释：
```
/**** libstdc++-v3 note:  Inclusion of this file has been removed from
 * all of the other STL headers for safety reasons, except std_utility.h.
 * For more information, see the thread of about twenty messages starting
 * with <URL:http://gcc.gnu.org/ml/libstdc++/2001-01/msg00223.html>, or the
 * FAQ at <URL:http://gcc.gnu.org/onlinedocs/libstdc++/faq/index.html#4_4>.
 *
 * Short summary:  the rel_ops operators cannot be made to play nice.
 * Don't use them.
*/
```
##pair
pair定义于stl_pair.h中。
```cpp
template <class _T1, class _T2>
struct pair
{
    typedef _T1 first_type;
    typedef _T2 second_type;

    _T1 first;
    _T2 second;
    
    pair() : first(_T1()), second(_T2()) {}
    pair(const _T1& __a, const _T2& __b) : first(__a), second(__b) {}
    template <class _U1, class _U2>
    pair(const pair<_U1, _U2>& __p) : first(__p.first), second(__p.second) {}
};

template <class _T1, class _T2>
inline bool operator==(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return __x.first == __y.first && __x.second == __y.second;
}

template <class _T1, class _T2>
inline bool operator<(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return __x.first < __y.first ||
           (!(__y.first < __x.first) && __x.second < __y.second);
}

template <class _T1, class _T2>
inline bool operator!=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return !(__x == __y);
}

template <class _T1, class _T2>
inline bool operator>(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return __y < __x;
}

template <class _T1, class _T2>
inline bool operator<=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return !(__y < __x);
}

template <class _T1, class _T2>
inline bool operator>=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return !(__x < __y);
}

template <class _T1, class _T2>
inline pair<_T1, _T2> make_pair(const _T1& __x, const _T2& __y)
{
    return pair<_T1, _T2>(__x, __y);
}
```
pair只是对两种类型进行了简单的封装，并定义了比较运算符而已。
另外，添加了一个辅助函数make_pair()来构造一个pair。