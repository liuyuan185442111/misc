STL中大部分算法定义于&lt;algorithms>，此头文件定义了一系列用于元素序列上的函数。
几乎所有泛型算法的前两个参数都是一对迭代器，用以表示算法的操作区间。

STL标准并没有区分基本算法或复杂算法，然而sgi却把常用的一些算法定义于stl_algobase.h中，其他算法定义于stl_algo.h中。
stl_algobase.h中定义的算法有（我有意去掉了很多与理解无关的东西）：
```cpp
template <typename ForwardIter1, typename ForwardIter2>
inline void iter_swap(ForwardIter1 a, ForwardIter2 b)
{
    typename iterator_traits<ForwardIter1>::value_type tmp = *a;
    *a = *b;
    *b = tmp;
}

// iter_swap的两个参数类型应该相同, 而且应等价于swap(*a,*b), 所以我推荐使用swap, 不推荐用iter_swap

template <typename T>
inline void swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
inline const T& min(const T& a, const T& b)
{
    return b < a ? b : a;
}

template <typename T>
inline const T& max(const T& a, const T& b)
{
    return  a < b ? b : a;
}

template <typename T, typename Compare>
inline const T& min(const T& a, const T& b, Compare comp)
{
    return comp(b, a) ? b : a;
}

template <typename T, typename Compare>
inline const T& max(const T& a, const T& b, Compare comp)
{
    return comp(a, b) ? b : a;
}

template <typename ForwardIter, typename T>
void fill(ForwardIter first, ForwardIter last, const T& value)
{
    for ( ; first != last; ++first)
        *first = value;
}

template <typename OutputIter, typename Size, typename T>
OutputIter fill_n(OutputIter first, Size n, const T& value)
{
    for ( ; n > 0; --n, ++first)
        *first = value;
    return first;
}

template <typename InputIter1, typename InputIter2>
pair<InputIter1, InputIter2>
mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2)
{
    while (first1 != last1 && *first1 == *first2)
    {
        ++first1;
        ++first2;
    }
    return pair<InputIter1, InputIter2>(first1, first2);
}

template <typename InputIter1, typename InputIter2, typename BinaryPredicate>
pair<InputIter1, InputIter2>
mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2, BinaryPredicate binary_pred)
{
    while (first1 != last1 && binary_pred(*first1, *first2))
    {
        ++first1;
        ++first2;
    }
    return pair<InputIter1, InputIter2>(first1, first2);
}

template <typename InputIter1, typename InputIter2>
inline bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2)
{
    for ( ; first1 != last1; ++first1, ++first2)
        if (!(*first1 == *first2))
            return false;
    return true;
}

template <typename InputIter1, typename InputIter2, typename BinaryPredicate>
inline bool equal(InputIter1 first1, InputIter1 last1,
                  InputIter2 first2, BinaryPredicate binary_pred)
{
    for ( ; first1 != last1; ++first1, ++first2)
        if (!binary_pred(*first1, *first2))
            return false;
    return true;
}

template <typename InputIter1, typename InputIter2>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
                             InputIter2 first2, InputIter2 last2)
{
    for ( ; first1 != last1 && first2 != last2
            ; ++first1, ++first2)
    {
        if (*first1 < *first2)
            return true;
        if (*first2 < *first1)
            return false;
    }
    return first1 == last1 && first2 != last2;
}

template <typename InputIter1, typename InputIter2, typename Compare>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1,
                             InputIter2 first2, InputIter2 last2, Compare comp)
{
    for ( ; first1 != last1 && first2 != last2
            ; ++first1, ++first2)
    {
        if (comp(*first1, *first2))
            return true;
        if (comp(*first2, *first1))
            return false;
    }
    return first1 == last1 && first2 != last2;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
{
    while (first!=last)
    {
        *result = *first;
        ++result;
        ++first;
    }
    return result;
}

template <typename BidirectionalIterator1, typename BidirectionalIterator2>
BidirectionalIterator2
copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
              BidirectionalIterator2 result)
{
    while (last!=first) *(--result) = *(--last);
    return result;
}
```
sgi为了提高效率，很多算法都有特化版本，然而我为了简单起见，将所有特化版本都删除了，下面以copy的完整版本为例说明。

	template <typename InputIter, typename OutputIter>
	inline OutputIter copy(InputIter first, InputIter last, OutputIter result);
```flow
st=>start: 开始
op1=>operation: 如果InputIter是normal_iterator, 提取出first和last的base()
op2=>operation: 如果OutputIter是normal_iterator, 提取出result的base()
op3=>operation: 提取出first所指向的型别T
cd1=>condition: T has_trivial_assignment_operator?
cd2=>condition: InputIter is pointer?
op4=>operation: use __copy_trivial
ed=>end: 结束
cd3=>condition: first is random_access_iterator?
op5=>operation: use 用整数进行循环计数的__copy
op6=>operation: use 最普通的__copy

st->op1->op2->op3->cd1
cd1(yes)->cd2
cd2(yes)->op4->ed
cd1(no)->cd3
cd2(no)->cd3
cd3(yes)->op5->ed
cd3(no)->op6->ed
```
```cpp
// copy.h
/**
所有的这些辅助函数有两个目的:
1.如果可能则使用memmove(非memcpy, memmove允许输入输出序列重叠);
2.如果原序列使用随机访问迭代器, 使用整数计数而不是通过比较迭代器来终止循环.
*/

// __normal_iterator将普通指针封装起来, 在stl_iterator.h中定义
// __type_traits用来提取type的特性, 在type_traits.h中定义
// __value_type和__distance_type在stl_iterator_base_types.h中定义

/**
为了测试, 我添加了一些输出语句.
因我是在gcc4.7.1中测试的, 需要将type_traits.h拷贝过来一起编译,
且需对其做些改动, 改动的地方在下面type_traits.h中说明.
*/
// 另外还将__value_type和__distance_type从stl_iterator_base_types.h中拷贝过来.

template <class Iter>
inline typename iterator_traits<Iter>::value_type*
__value_type(const Iter&)
{
    return static_cast<typename iterator_traits<Iter>::value_type*>(0);
}

template <class Iter>
inline typename iterator_traits<Iter>::difference_type*
__distance_type(const Iter&)
{
    return static_cast<typename iterator_traits<Iter>::difference_type*>(0);
}

// copy的真正实现
template <typename InputIter, typename OutputIter, typename Distance>
inline OutputIter __copy(InputIter first, InputIter last,
                         OutputIter result,
                         input_iterator_tag, Distance*)
{
    cout << "copy: with iterator.\n";
    for ( ; first != last; ++result, ++first)
        *result = *first;
    return result;
}

// 对随机访问迭代器的特化
template <typename RandomAccessIter, typename OutputIter, typename Distance>
inline OutputIter
__copy(RandomAccessIter first, RandomAccessIter last,
       OutputIter result, random_access_iterator_tag, Distance*)
{
    cout << "copy: with int.\n";
    for (Distance n = last - first; n > 0; --n)
    {
        *result = *first;
        ++first;
        ++result;
    }
    return result;
}

// 对普通指针的特化
template <typename T>
inline T*
__copy_trivial(const T* first, const T* last, T* result)
{
    cout << "copy_trivial.\n";
    memmove(result, first, sizeof(T) * (last - first));
    return result + (last - first);
}


template <typename InputIter, typename OutputIter>
inline OutputIter __copy_aux2(InputIter first, InputIter last,
                              OutputIter result, __false_type)
{
    cout << "copy_aux2: *InputIter is not trivial.\n";
    return __copy(first, last, result,
                  __iterator_category(first),
                  __distance_type(first));
}

template <typename InputIter, typename OutputIter>
inline OutputIter __copy_aux2(InputIter first, InputIter last,
                              OutputIter result, __true_type)
{
    cout << "copy_aux2: *InputIter is trivial.\n";
    return __copy(first, last, result,
                  __iterator_category(first),
                  __distance_type(first));
}

template <typename T>
inline T* __copy_aux2(T* first, T* last, T* result, __true_type)
{
    cout << "copy_aux2: InputIter is T*.\n";
    return __copy_trivial(first, last, result);
}

template <typename T>
inline T* __copy_aux2(const T* first, const T* last, T* result, __true_type)
{
    cout << "copy_aux2: InputIter is const T*.\n";
    return __copy_trivial(first, last, result);
}


template <typename InputIter, typename OutputIter, typename T>
inline OutputIter __copy_aux(InputIter first, InputIter last,
                             OutputIter result, T*)
{
    cout << "copy_aux.\n";
    typedef typename __type_traits<T>::has_trivial_assignment_operator Trivial;
    return __copy_aux2(first, last, result, Trivial());
}

template<typename InputIter, typename OutputIter>
inline OutputIter __copy_ni2(InputIter first, InputIter last,
                             OutputIter result, __true_type)
{
    cout << "copy_ni2: OutputIter is normal_iterator.\n";
    return OutputIter(__copy_aux(first, last, result.base(), __value_type(first)));
}

template<typename InputIter, typename OutputIter>
inline OutputIter __copy_ni2(InputIter first, InputIter last,
                             OutputIter result, __false_type)
{
    cout << "copy_ni2: OutputIter is not normal_iterator.\n";
    return __copy_aux(first, last, result, __value_type(first));
}

template<typename InputIter, typename OutputIter>
inline OutputIter __copy_ni1(InputIter first, InputIter last,
                             OutputIter result, __true_type)
{
    cout << "copy_ni1: InputIter is normal_iterator.\n";
    typedef typename _Is_normal_iterator<OutputIter>::_Normal Normal;
    return __copy_ni2(first.base(), last.base(), result, Normal());
}

template<typename InputIter, typename OutputIter>
inline OutputIter __copy_ni1(InputIter first, InputIter last,
                             OutputIter result, __false_type)
{
    cout << "copy_ni1: InputIter is not normal_iterator.\n";
    typedef typename _Is_normal_iterator<OutputIter>::_Normal Normal;
    return __copy_ni2(first, last, result, Normal());
}

// 因copy已在库中定义, 我便改了个名字
template <typename InputIter, typename OutputIter>
inline OutputIter mycopy(InputIter first, InputIter last, OutputIter result)
{
    cout << "mycopy.\n";
    typedef typename _Is_normal_iterator<InputIter>::_Normal Normal;
    return __copy_ni1(first, last, result, Normal());
}
```
修改后的type_traits.h（与原版本相比做了很多删减，和一些修改）：
```cpp
// type_traits.h
// __true_type和__false_type已在其他地方定义, 因此将其从原版文件中删掉
// __normal_iterator定义在命名空间__gnu_cxx中, 而非旧版本的std, 在此文件的结尾将其修改
template <class _Tp>
struct __type_traits
{
    typedef __false_type    has_trivial_default_constructor;
    typedef __false_type    has_trivial_copy_constructor;
    typedef __false_type    has_trivial_assignment_operator;
    typedef __false_type    has_trivial_destructor;
    typedef __false_type    is_POD_type;
};

template<> struct __type_traits<int>
{
    typedef __true_type    has_trivial_default_constructor;
    typedef __true_type    has_trivial_copy_constructor;
    typedef __true_type    has_trivial_assignment_operator;
    typedef __true_type    has_trivial_destructor;
    typedef __true_type    is_POD_type;
};

template <class _Tp>
struct __type_traits<_Tp*>
{
    typedef __true_type    has_trivial_default_constructor;
    typedef __true_type    has_trivial_copy_constructor;
    typedef __true_type    has_trivial_assignment_operator;
    typedef __true_type    has_trivial_destructor;
    typedef __true_type    is_POD_type;
};

template<typename _Tp> struct _Is_normal_iterator
{
    typedef __false_type _Normal;
};

template<typename _Iterator, typename _Container>
struct _Is_normal_iterator< __gnu_cxx::__normal_iterator<_Iterator, _Container> >
{
    typedef __true_type _Normal;
};
```
测试源文件：
```cpp
#include <iostream>
#include <vector>
#include <iterator>
#include <cstring>
#include <deque>
using namespace std;
#include "type_traits.h"
#include "copy.h"

struct myint
{
    myint(int m=0):i(m){}
    myint &operator=(myint m){i=m.i;return *this;}
    int i;
};

// 为__type_traits做一个特化
template<> struct __type_traits<myint> {
   typedef __true_type    has_trivial_default_constructor;
   typedef __true_type    has_trivial_copy_constructor;
   typedef __true_type    has_trivial_assignment_operator;
   typedef __true_type    has_trivial_destructor;
   typedef __true_type    is_POD_type;
};

int main()
{
    int ia[] = {1,2,3};
    int iaa[3];
    vector<int> iv(ia,ia+3);
    vector<int> ivv(3);
    // vector的迭代器是normal_iterator
    int *a = iv.begin().base();
    int *b = iv.end().base();
    int *c = ivv.begin().base();
    mycopy(a,b,c);

    cout << "----------------------------\n";
    mycopy(iv.begin(),iv.end(),ivv.begin());

    cout << "----------------------------\n";
    mycopy(ia,ia+3,iaa);

    cout << "----------------------------\n";
    deque<int> de(ia,ia+3);
    deque<int> dee(3);
    mycopy(de.begin(),de.end(),dee.begin());

    cout << "----------------------------\n";
    vector<myint> mi(ia,ia+3);
    vector<myint> mii(3);
    mycopy(mi.begin(),mi.end(),mii.begin());
    cout << mii[0].i << endl;

    cout << "----------------------------\n";
    deque<myint> dd(ia,ia+3);
    deque<myint> ddd(3);
    mycopy(dd.begin(),dd.end(),ddd.begin());
    cout << ddd.begin()->i;

    return 0;
}
```