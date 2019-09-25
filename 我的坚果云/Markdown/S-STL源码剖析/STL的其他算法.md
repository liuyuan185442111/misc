在[STL的基础算法](http://blog.csdn.net/liuyuan185442111/article/details/46553499)中说明了一些STL的一些基础算法，剩下的内容在下面说明。
##迭代器
每个STL算法的声明，都表现出他所需要的最低程度的迭代器类型。例如find()需要一个InputIterator，这是它的最低要求，但它也可以接受更高类型的迭代器，如ForwardIterator，BidirectionIterator或RandomAccessIterator。
##接受函数对象
许多STL算法不只支持一个版本。这一类算法的某个版本采用缺省运算行为，另一个版本提供额外参数，接受外界传入一个函数对象，以便采用其他策略，例如unique()。有些算法干脆将这样的两个版本分为不同名称的实体，附从的那个以_if作为结尾，例如find_if()。

采用重载函数的那些算法接受的函数对象类型被声明为BinaryPredicate，它接受两个参数，返回true或false，通常具有判断两个参数是否相等的语义。

以if结尾的算法需要的函数对象类型被声明为Predicate或UnaryPredicate，它接受一个参数，返回true或false，通常具有判断参数是否满足特定条件的语义。
##质变算法
质变算法（mutating algorithms）通常提供两个版本：一个是in-place版，就地改变其操作对象；另一个是copy版，将修改后的操作对象复制到它处。copy版总是以_copy作为函数名称的结尾，例如replace()和replace_copy()。并不是所有的质变算法都有copy版，例如sort()就没有。
##关于删除元素
如果经STL算法处理后，序列元素会减少，比如remove，unique，序列实际大小并没有减少，需要调用容器的erase放来来删除序列结尾的多余元素。
##简单遍历
count，count_if
min_element，max_element
for_each，transform，generate
generate_n
```cpp
/**************** 简单遍历 ****************/

// 统计序列中等于val的元素的数目
template <class InputIterator, class T>
typename iterator_traits<InputIterator>::difference_type
count (InputIterator first, InputIterator last, const T& val)
{
    typename iterator_traits<InputIterator>::difference_type ret = 0;
    while (first!=last)
    {
        if (*first == val) ++ret;
        ++first;
    }
    return ret;
}

template <class InputIterator, class UnaryPredicate>
typename iterator_traits<InputIterator>::difference_type
count_if (InputIterator first, InputIterator last, UnaryPredicate pred)
{
    typename iterator_traits<InputIterator>::difference_type ret = 0;
    while (first!=last)
    {
        if (pred(*first)) ++ret;
        ++first;
    }
    return ret;
}

// 返回序列中最小元素的位置
template <class ForwardIterator>
ForwardIterator min_element ( ForwardIterator first, ForwardIterator last )
{
    if (first==last) return last;
    ForwardIterator smallest = first;

    while (++first!=last)
        if (*first<*smallest)
            smallest=first;
    return smallest;
}
template <class ForwardIterator, class Compare>
ForwardIterator min_element (ForwardIterator first, ForwardIterator last, Compare comp);

// 返回序列中最大元素的位置
template <class ForwardIterator>
ForwardIterator max_element (ForwardIterator first, ForwardIterator last);
template <class ForwardIterator, class Compare>
ForwardIterator max_element (ForwardIterator first, ForwardIterator last, Compare comp);

// 对序列的每个元素调用fn
template<class InputIterator, class Function>
Function for_each(InputIterator first, InputIterator last, Function fn)
{
    while (first!=last)
    {
        fn (*first);
        ++first;
    }
    return fn;
}

// 对序列的每个元素调用op, 将结果写入到新序列中
template <class InputIterator, class OutputIterator, class UnaryOperator>
OutputIterator transform (InputIterator first1, InputIterator last1,
                          OutputIterator result, UnaryOperator op)
{
    while (first1 != last1)
    {
        *result = op(*first1);
        ++result;
        ++first1;
    }
    return result;
}

template <class InputIterator1, class InputIterator2,
         class OutputIterator, class BinaryOperation>
OutputIterator transform (InputIterator1 first1, InputIterator1 last1,
                          InputIterator2 first2, OutputIterator result,
                          BinaryOperation binary_op)
{
    while (first1 != last1)
    {
        *result=binary_op(*first1,*first2++);
        ++result;
        ++first1;
    }
    return result;
}

// 为序列元素赋值
template <class ForwardIterator, class Generator>
void generate ( ForwardIterator first, ForwardIterator last, Generator gen )
{
    while (first != last)
    {
        *first = gen();
        ++first;
    }
}

template <class OutputIterator, class Size, class Generator>
void generate_n ( OutputIterator first, Size n, Generator gen )
{
    while (n>0)
    {
        *first = gen();
        ++first;
        --n;
    }
}
```
##二分查找
lower_bound，upper_bound，equal_range，binary_search 
```cpp
/**************** 二分查找 ****************/

// 返回序列中第一个不小于val的元素
template <class ForwardIterator, class T>
ForwardIterator lower_bound (ForwardIterator first, ForwardIterator last, const T& val);
template <class ForwardIterator, class T, class Compare>
ForwardIterator lower_bound (ForwardIterator first, ForwardIterator last, const T& val, Compare comp);

// 返回序列中第一个大于val的元素
template <class ForwardIterator, class T>
ForwardIterator upper_bound (ForwardIterator first, ForwardIterator last, const T& val);
template <class ForwardIterator, class T, class Compare>
ForwardIterator upper_bound (ForwardIterator first, ForwardIterator last, const T& val, Compare comp);

// 等价于make_pair(lower_bound(first,last,val), upper_bound(first,last,val));
// 但效率会更高
template <class ForwardIterator, class T>
pair<ForwardIterator,ForwardIterator>
equal_range (ForwardIterator first, ForwardIterator last, const T& val)
{
    ForwardIterator it = std::lower_bound (first,last,val);
    return std::make_pair ( it, std::upper_bound(it,last,val) );
}

template <class ForwardIterator, class T, class Compare>
pair<ForwardIterator,ForwardIterator>
equal_range (ForwardIterator first, ForwardIterator last, const T& val, Compare comp);

// 二分查找序列中第一个与val等价的元素
template <class ForwardIterator, class T>
bool binary_search (ForwardIterator first, ForwardIterator last, const T& val)
{
    first = std::lower_bound(first,last,val);
    return (first!=last && !(val<*first));
}

template <class ForwardIterator, class T, class Compare>
bool binary_search (ForwardIterator first, ForwardIterator last, const T& val, Compare comp);
```
##部分排序
```cpp
/**************** 部分排序 ****************/
/**使用堆排序, 排序完毕, [first,middle)有序, [middle,last)不一定有序
首先利用max_heap()将[first,Middle)组织成一个max-heap,
然后将[middle,last)中的每一个元素与max-heap的最大值比较,
如果小于该最大值, 就互换位置并重新保持max-heap的状态,
当走遍[middle,last)时, 较大的元素都已经抽离出[first,middle),
再以sort_heap()将[first,middle)做一次排序即可.
*/
template <class RandomAccessIterator>
void partial_sort (RandomAccessIterator first, RandomAccessIterator middle,
                   RandomAccessIterator last);
template <class RandomAccessIterator, class Compare>
void partial_sort (RandomAccessIterator first, RandomAccessIterator middle,
                   RandomAccessIterator last, Compare comp);

template <class InputIterator, class RandomAccessIterator>
  RandomAccessIterator
    partial_sort_copy (InputIterator first,InputIterator last,
                       RandomAccessIterator result_first,
                       RandomAccessIterator result_last);

template <class InputIterator, class RandomAccessIterator, class Compare>
  RandomAccessIterator
    partial_sort_copy (InputIterator first,InputIterator last,
                       RandomAccessIterator result_first,
                       RandomAccessIterator result_last, Compare comp);
```
##random_shuffle
```cpp
/**************** random_shuffle ****************/
template <class _RandomAccessIter>
inline void random_shuffle(_RandomAccessIter __first, _RandomAccessIter __last)
{
    if (__first == __last) return;
    for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
        iter_swap(__i, __first + rand() % ((__i - __first) + 1));
}

// 随机数产生器通过引用传递, 因为每次重新构造一个随机数产生器, 产生的随机序列可能就会相同
template <class _RandomAccessIter, class _RandomNumberGenerator>
void random_shuffle(_RandomAccessIter __first, _RandomAccessIter __last,
                    _RandomNumberGenerator& __rand)
{
    if (__first == __last) return;
    for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
        iter_swap(__i, __first + __rand((__i - __first) + 1));
}
```
##交换
```cpp
// 交换两个序列
template <class ForwardIterator1, class ForwardIterator2>
ForwardIterator2 swap_ranges (ForwardIterator1 first1, ForwardIterator1 last1,
                              ForwardIterator2 first2)
{
    while (first1!=last1)
    {
        swap (*first1, *first2);
        ++first1;
        ++first2;
    }
    return first2;
}

// 逆序序列
template <class BidirectionalIterator>
void reverse (BidirectionalIterator first, BidirectionalIterator last)
{
    while ((first!=last)&&(first!=--last))
    {
        std::iter_swap (first,last);
        ++first;
    }
}

template <class BidirectionalIterator, class OutputIterator>
OutputIterator reverse_copy (BidirectionalIterator first,
                             BidirectionalIterator last, OutputIterator result)
{
    while (first!=last)
    {
        --last;
        *result = *last;
        ++result;
    }
    return result;
}

// 翻转序列, 交换序列[first,middle)和[middle,last)
template <class ForwardIterator>
void rotate (ForwardIterator first, ForwardIterator middle,
             ForwardIterator last)
{
    ForwardIterator next = middle;
    while (first!=next)
    {
        swap (*first++,*next++);
        if (next==last) next=middle;
        else if (first==middle) middle=next;
    }
}

template <class ForwardIterator, class OutputIterator>
OutputIterator rotate_copy (ForwardIterator first, ForwardIterator middle,
                            ForwardIterator last, OutputIterator result)
{
    result=std::copy (middle,last,result);
    return std::copy (first,middle,result);
}
```
##两个有序序列的操作
merge，set_union，set_intersection，set_difference，set_symmetric_difference，它们都是稳定的
includes
```cpp
// 将两个有序序列合并为一个
template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator merge (InputIterator1 first1, InputIterator1 last1,
                      InputIterator2 first2, InputIterator2 last2,
                      OutputIterator result);


template <class InputIterator1, class InputIterator2,
         class OutputIterator, class Compare>
OutputIterator merge (InputIterator1 first1, InputIterator1 last1,
                      InputIterator2 first2, InputIterator2 last2,
                      OutputIterator result, Compare comp);


// 可接受STL的set/multiset，不可接受hash_set/hash_multiset
// 并集, 如果某个值在s1中出现m次, 在s2中出现n次, 那么该值在输出区间中会出现max(m,n)次, 其中m个来自s1, 其余来自s2
template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_union(_InputIter1 __first1, _InputIter1 __last1,
                      _InputIter2 __first2, _InputIter2 __last2,
                      _OutputIter __result)
{
    while (__first1 != __last1 && __first2 != __last2)
    {
        if (*__first1 < *__first2)
        {
            *__result = *__first1;
            ++__first1;
        }
        else if (*__first2 < *__first1)
        {
            *__result = *__first2;
            ++__first2;
        }
        else
        {
            *__result = *__first1;
            ++__first1;
            ++__first2;
        }
        ++__result;
    }
    return copy(__first2, __last2, copy(__first1, __last1, __result));
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
         class _Compare>
_OutputIter set_union(_InputIter1 __first1, _InputIter1 __last1,
                      _InputIter2 __first2, _InputIter2 __last2,
                      _OutputIter __result, _Compare __comp);


// 交集
// 如果某个值在s1中出现m次, 在s2中出现n次, 那么该值在输出区间中会出现min(m,n)次, 全部来自s1
template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_intersection(_InputIter1 __first1, _InputIter1 __last1,
                             _InputIter2 __first2, _InputIter2 __last2,
                             _OutputIter __result)
{
    while (__first1 != __last1 && __first2 != __last2)
        if (*__first1 < *__first2)
            ++__first1;
        else if (*__first2 < *__first1)
            ++__first2;
        else
        {
            *__result = *__first1;
            ++__first1;
            ++__first2;
            ++__result;
        }
    return __result;
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
         class _Compare>
_OutputIter set_intersection(_InputIter1 __first1, _InputIter1 __last1,
                             _InputIter2 __first2, _InputIter2 __last2,
                             _OutputIter __result, _Compare __comp);

// 差集
// 如果某个值在s1中出现m次, 在s2中出现n次, 那么该值在输出区间中会出现max(m-n,0)次, 全部来自s1
template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_difference(_InputIter1 __first1, _InputIter1 __last1,
                           _InputIter2 __first2, _InputIter2 __last2,
                           _OutputIter __result)
{
    while (__first1 != __last1 && __first2 != __last2)
        if (*__first1 < *__first2)
        {
            *__result = *__first1;
            ++__first1;
            ++__result;
        }
        else if (*__first2 < *__first1)
            ++__first2;
        else
        {
            ++__first1;
            ++__first2;
        }
    return copy(__first1, __last1, __result);
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
         class _Compare>
_OutputIter set_difference(_InputIter1 __first1, _InputIter1 __last1,
                           _InputIter2 __first2, _InputIter2 __last2,
                           _OutputIter __result, _Compare __comp);


// 对称差集
// (s1-s2)∪(s2-s1)
// 如果某个值在s1中出现m次, 在s2中出现n次, 那么该值在输出区间中会出现|m-n|次, 如果m>n, 来自s1;如果n>m,来自s2
template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter
set_symmetric_difference(_InputIter1 __first1, _InputIter1 __last1,
                         _InputIter2 __first2, _InputIter2 __last2,
                         _OutputIter __result)
{
    while (__first1 != __last1 && __first2 != __last2)
        if (*__first1 < *__first2)
        {
            *__result = *__first1;
            ++__first1;
            ++__result;
        }
        else if (*__first2 < *__first1)
        {
            *__result = *__first2;
            ++__first2;
            ++__result;
        }
        else
        {
            ++__first1;
            ++__first2;
        }
    return copy(__first2, __last2, copy(__first1, __last1, __result));
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
         class _Compare>
_OutputIter
set_symmetric_difference(_InputIter1 __first1, _InputIter1 __last1,
                         _InputIter2 __first2, _InputIter2 __last2,
                         _OutputIter __result,
                         _Compare __comp);

template <class InputIterator1, class InputIterator2>
  bool includes ( InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, InputIterator2 last2 );

template <class InputIterator1, class InputIterator2, class Compare>
  bool includes ( InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, InputIterator2 last2, Compare comp );
```
##查找
find，find_if，find_end*，find_first_of*，adjacent_find*
unique，unique_copy
search*，search_n*序列
```cpp
// 查找序列2在序列1中第一次出现的位置
template <class ForwardIterator1, class ForwardIterator2>
ForwardIterator1 search (ForwardIterator1 first1, ForwardIterator1 last1,
                         ForwardIterator2 first2, ForwardIterator2 last2);
template <class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
ForwardIterator1 search (ForwardIterator1 first1, ForwardIterator1 last1,
                         ForwardIterator2 first2, ForwardIterator2 last2,
                         BinaryPredicate pred);

// 查找序列中连续count个val第一次出现的位置
template <class ForwardIterator, class Size, class T>
ForwardIterator search_n (ForwardIterator first, ForwardIterator last,
                          Size count, const T& val);
template <class ForwardIterator, class Size, class T, class BinaryPredicate>
ForwardIterator search_n ( ForwardIterator first, ForwardIterator last,
                           Size count, const T& val, BinaryPredicate pred );

// 查找序列中与val相等的第一个元素的位置
template<class InputIterator, class T>
InputIterator find (InputIterator first, InputIterator last, const T& val)
{
    while (first!=last)
    {
        if (*first==val) return first;
        ++first;
    }
    return last;
}

template <class InputIterator, class UnaryPredicate>
InputIterator find_if (InputIterator first, InputIterator last, UnaryPredicate pred);

// 查找序列2在序列1中最后一次出现的位置
template <class ForwardIterator1, class ForwardIterator2>
ForwardIterator1 find_end (ForwardIterator1 first1, ForwardIterator1 last1,
                           ForwardIterator2 first2, ForwardIterator2 last2);
template <class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
ForwardIterator1 find_end (ForwardIterator1 first1, ForwardIterator1 last1,
                           ForwardIterator2 first2, ForwardIterator2 last2,
                           BinaryPredicate pred);

// 查找序列2中任一元素在序列1中第一次出现的位置
template <class ForwardIterator1, class ForwardIterator2>
ForwardIterator1 find_first_of (ForwardIterator1 first1, ForwardIterator1 last1,
                                ForwardIterator2 first2, ForwardIterator2 last2);

template <class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
ForwardIterator1 find_first_of (ForwardIterator1 first1, ForwardIterator1 last1,
                                ForwardIterator2 first2, ForwardIterator2 last2,
                                BinaryPredicate pred);

// 查找序列中第一次出现连续两个元素相等的位置
template <class ForwardIterator>
ForwardIterator adjacent_find (ForwardIterator first, ForwardIterator last)
{
  if (first != last)
  {
    ForwardIterator next=first; ++next;
    while (next != last) {
      if (*first == *next)     // or: if (pred(*first,*next)), for version (2)
        return first;
      ++first; ++next;
    }
  }
  return last;
}

template <class ForwardIterator, class BinaryPredicate>
ForwardIterator adjacent_find (ForwardIterator first, ForwardIterator last,
                               BinaryPredicate pred);

// 将连续若干个相等元素中除第一个外的元素删除
template <class ForwardIterator>
ForwardIterator unique (ForwardIterator first, ForwardIterator last);

template <class ForwardIterator, class BinaryPredicate>
ForwardIterator unique (ForwardIterator first, ForwardIterator last,
                        BinaryPredicate pred);

template <class InputIterator, class OutputIterator>
OutputIterator unique_copy (InputIterator first, InputIterator last,
                            OutputIterator result);

template <class InputIterator, class OutputIterator, class BinaryPredicate>
OutputIterator unique_copy (InputIterator first, InputIterator last,
                            OutputIterator result, BinaryPredicate pred);
```
##替换
replace，replace_if，replace_copy，replace_copy_if
```cpp
// 将序列中所有等于old_value的元素的值赋为new_value
template <class ForwardIterator, class T>
void replace (ForwardIterator first, ForwardIterator last,
              const T& old_value, const T& new_value)
{
    while (first!=last)
    {
        if (*first == old_value) *first=new_value;
        ++first;
    }
}

template < class ForwardIterator, class UnaryPredicate, class T >
void replace_if (ForwardIterator first, ForwardIterator last,
                 UnaryPredicate pred, const T& new_value)
{
    while (first!=last)
    {
        if (pred(*first)) *first=new_value;
        ++first;
    }
}

template <class InputIterator, class OutputIterator, class T>
OutputIterator replace_copy (InputIterator first, InputIterator last,
                             OutputIterator result, const T& old_value, const T& new_value)
{
    while (first!=last)
    {
        *result = (*first==old_value)? new_value: *first;
        ++first;
        ++result;
    }
    return result;
}

template <class InputIterator, class OutputIterator, class UnaryPredicate, class T>
OutputIterator replace_copy_if (InputIterator first, InputIterator last,
                                OutputIterator result, UnaryPredicate pred,
                                const T& new_value)
{
    while (first!=last)
    {
        *result = (pred(*first))? new_value: *first;
        ++first;
        ++result;
    }
    return result;
}

```
##删除
remove，remove_if，remove_copy，remove_copy_if
```cpp
// 将序列中所有等于val的元素都删除
template <class ForwardIterator, class T>
ForwardIterator remove (ForwardIterator first, ForwardIterator last, const T& val)
{
    ForwardIterator result = first;
    while (first!=last)
    {
        if (!(*first == val))
        {
            *result = *first;
            ++result;
        }
        ++first;
    }
    return result;
}

template <class ForwardIterator, class UnaryPredicate>
ForwardIterator remove_if (ForwardIterator first, ForwardIterator last,
                           UnaryPredicate pred)
{
    ForwardIterator result = first;
    while (first!=last)
    {
        if (!pred(*first))
        {
            *result = *first;
            ++result;
        }
        ++first;
    }
    return result;
}

template <class InputIterator, class OutputIterator, class T>
OutputIterator remove_copy (InputIterator first, InputIterator last,
                            OutputIterator result, const T& val)
{
    while (first!=last)
    {
        if (!(*first == val))
        {
            *result = *first;
            ++result;
        }
        ++first;
    }
    return result;
}

template <class InputIterator, class OutputIterator, class UnaryPredicate>
OutputIterator remove_copy_if (InputIterator first, InputIterator last,
                               OutputIterator result, UnaryPredicate pred)
{
    while (first!=last)
    {
        if (!pred(*first))
        {
            *result = *first;
            ++result;
        }
        ++first;
    }
    return result;
}

```
##复杂的算法
还有一些复杂的算法，比如inplace_merge，stable_partition，stable_sort，不准备再看了。
sort，stable_sort，nth_element 
inplace_merge
partition，stable_partition
```cpp
// 就地合并
template <class BidirectionalIterator>
void inplace_merge (BidirectionalIterator first, BidirectionalIterator middle,
                    BidirectionalIterator last);

template <class BidirectionalIterator, class Compare>
void inplace_merge (BidirectionalIterator first, BidirectionalIterator middle,
                    BidirectionalIterator last, Compare comp);


// 将排好序后应处于nth位置的元素置于nth位置, 它之前的元素都小于等于它, 之后的元素都大于等于它, 但前后两个子序列不一定是有序的
template <class RandomAccessIterator>
void nth_element (RandomAccessIterator first, RandomAccessIterator nth,
                  RandomAccessIterator last);

template <class RandomAccessIterator, class Compare>
void nth_element (RandomAccessIterator first, RandomAccessIterator nth,
                  RandomAccessIterator last, Compare comp);

// 排序
template <class RandomAccessIterator>
void sort (RandomAccessIterator first, RandomAccessIterator last);

template <class RandomAccessIterator, class Compare>
void sort (RandomAccessIterator first, RandomAccessIterator last, Compare comp);

template <class RandomAccessIterator>
void stable_sort ( RandomAccessIterator first, RandomAccessIterator last );

template <class RandomAccessIterator, class Compare>
void stable_sort ( RandomAccessIterator first, RandomAccessIterator last,
                   Compare comp );

// 将序列分为两部分, 前半部分pred为true, 后半部分pred为false
template <class BidirectionalIterator, class UnaryPredicate>
BidirectionalIterator partition (BidirectionalIterator first,
                                 BidirectionalIterator last, UnaryPredicate pred)
{
    while (first!=last)
    {
        while (pred(*first))
        {
            ++first;
            if (first==last) return first;
        }
        do
        {
            --last;
            if (first==last) return first;
        }
        while (!pred(*last));
        swap (*first,*last);
        ++first;
    }
    return first;
}

template <class BidirectionalIterator, class UnaryPredicate>
BidirectionalIterator stable_partition (BidirectionalIterator first,
                                        BidirectionalIterator last,
                                        UnaryPredicate pred);
```
##STL的rotate算法
STL的rotate算法，针对迭代器的类型不同，有三种：
###对于前向迭代器，分组交换
若$a$长度大于$b$，将$ab$分成$a_0a_1b$，交换$a_0$和$b$，得$ba_1a_0$，只需再交换$a_1$和$a_0$。
若$a$长度小于$b$，将$ab$分成$ab_0b_1$，交换$a$和$b_0$，得$b_0ab_1$，只需再交换$a$ 和$b_1$。
不断进行划分和交换，直到不能再划分为止。
总共执行n/2到n次swap操作。
很容易用递归实现：
```
void forward_rotate(int *first, int *middle, int *last)
{
    if(first == middle || middle == last) return;
    int *next = middle;
    while(1)
    {
        swap(*first++, *next++);
        if(first == middle) {forward_rotate(first, next, last);return;}
        else if(next == last) {forward_rotate(first, middle, last);return;}
    }
}
```
依照递归版本写出迭代版本来：
```
void forward_rotate_2(int *first, int *middle, int *last)
{
    if(first == middle || middle == last) return;
    int *next = middle;
    while(first != middle && middle != last)
    {
        swap(*first++, *next++);
        if(first == middle) middle = next;
        else if(next == last) next = middle;
    }
}
```
当执行next=middle时, first已经不等于middle了, 所以可省略while中first!=middle的判断：
```
void forward_rotate_3(int *first, int *middle, int *last)
{
    if(first == middle || middle == last) return;
    int *next = middle;
    while(middle != last)
    {
        swap(*first++, *next++);
        if(first == middle) middle = next;
        else if(next == last) next = middle;
    }
}
```
###对于双向迭代器，三次反转
利用$ ba=(b^r)^r(a^r)^r=(a^rb^r)^r $，先分别反转a、b，最后再对所有元素进行一次反转。
总共执行了n次swap操作。
例如：
```cpp
void bid_rotate(int *first, int *middle, int *last)
{
    reverse(first, middle);
    reverse(middle, last);
    reverse(first, last);
}
```
###对于随机访问迭代器，一步到位
假定当区间为[0,1,2,3,4,5,6,7,8]，[first,middle)为[0,1,2]，[middle,last)为[3,4,5,6,7,8]。
算法如下：
先将0取出，将3放入0的位置，将6放入3的位置，将0放入6的位置。
依次进行即可。
###参考
SGI STL笔记（rotate算法介绍）
http://blog.chinaunix.net/uid-10647744-id-3274208.html
数组左旋转k位 —— C++标准算法库中最悲剧的函数：rotate
http://www.cnblogs.com/flyinghearts/archive/2011/05/27/2060265.html
注：并不像文中说的那样，随机迭代器版本的rotate速度最快！
##lower_bound和upper_bound
[cplusplus.com](http://www.cplusplus.com/reference/algorithm/lower_bound/)上给出的实现：
```
template <class ForwardIterator, class T>
  ForwardIterator lower_bound (ForwardIterator first, ForwardIterator last, const T& val)
{
  ForwardIterator it;
  iterator_traits<ForwardIterator>::difference_type count, step;
  count = distance(first,last);
  while (count>0)
  {
    it = first; step=count/2; advance (it,step);
    if (*it<val) {
      first=++it;
      count-=step+1;
    }
    else count=step;
  }
  return first;
}

template <class ForwardIterator, class T>
  ForwardIterator upper_bound (ForwardIterator first, ForwardIterator last, const T& val)
{
  ForwardIterator it;
  iterator_traits<ForwardIterator>::difference_type count, step;
  count = std::distance(first,last);
  while (count>0)
  {
    it = first; step=count/2; std::advance (it,step);
    if (!(val<*it))
      { first=++it; count-=step+1;  }
    else count=step;
  }
  return first;
}
```