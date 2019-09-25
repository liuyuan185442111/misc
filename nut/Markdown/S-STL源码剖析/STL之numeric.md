头文件&lt;numeric>在数值序列上定义了一组一般数学操作，也可以用于其他序列。
有四个函数模板：
|name|description|
|-|-|
|accumulate|累加和|
|adjacent_difference|相邻元素之差|
|inner_product|累加内积|
|partial_sum|部分和|
对每个操作，你都可以自定义“和”，“差”，“积”的行为，然后传递给函数执行操作的对象。
##名词说明
int numbers[] = {10,20,30};
累加和：10+20+30

x表示[first,last)中元素，y表示结果序列中元素，相邻元素之差：
y0 = x0 
y1 = x1 - x0 
y2 = x2 - x1 
y3 = x3 - x2 
y4 = x4 - x3 
... ... ...

int series1[] = {10,20,30};
int series2[] = {1,2,3};
累加内积：10*1+20*2+30*3

x表示[first,last)中元素，y表示结果序列中元素，部分和：
y0 = x0 
y1 = x0 + x1 
y2 = x0 + x1 + x2 
y3 = x0 + x1 + x2 + x3 
y4 = x0 + x1 + x2 + x3 + x4 
... ... ...
##累加
先看累加和，累加内积（stl_numeric.h）：
```cpp
template <typename InputIterator, typename T>
T accumulate(InputIterator first, InputIterator last, T init)
{
    for ( ; first != last; ++first)
        init = init + *first;
    return init;
}

template <typename InputIterator, typename T, typename BinaryOperation>
T accumulate(InputIterator first, InputIterator last, T init,
             BinaryOperation binary_op)
{
    for ( ; first != last; ++first)
        init = binary_op(init, *first);
    return init;
}

template <typename InputIterator1, typename InputIterator2, typename T>
T inner_product(InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, T init)
{
    for ( ; first1 !=last1; ++first1, ++first2)
        init = init + (*first1 **first2);
    return init;
}

template <typename InputIterator1, typename InputIterator2, typename T,
         typename BinaryOperation1, typename BinaryOperation2>
T inner_product(InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, T init,
                BinaryOperation1 binary_op1,
                BinaryOperation2 binary_op2)
{
    for ( ; first1 != last1; ++first1, ++first2)
        init = binary_op1(init, binary_op2(*first1, *first2));
    return init;
}
```
##部分和
在stl_numeric.h中实现：
```cpp
template <class InputIterator, class  OutputIterator, typename T>
OutputIterator
__partial_sum(InputIterator first, InputIterator last,
              OutputIterator result, T*)
{
    T value = *first;
    while(++first != last)
    {
        value = value + *first;
        *++result = value;
    }
    return ++result;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator
partial_sum(InputIterator first, InputIterator last,
            OutputIterator result)
{
    if(first == last) return result;
    *result = *first;
    return __partial_sum(first, last, result, value_type(first));
}

template <typename InputIterator, typename OutputIterator, typename T,
         typename BinaryOperation>
OutputIterator
__partial_sum(InputIterator first, InputIterator last,
              OutputIterator result, T*, BinaryOperation binary_op)
{
    T value = *first;
    while (++first != last)
    {
        value = binary_op(value, *first);
        *++result =value;
    }
    return ++result;
}

template <typename InputIterator, typename OutputIterator, typename BinaryOperation>
OutputIterator
partial_sum(InputIterator first, InputIterator last,
            OutputIterator result, BinaryOperation binary_op)
{
    if(first == last) return result;
    *result = *first;
    return __partial_sum(first, last, result, value_type(first), binary_op);
}
```
##相邻元素之差
```cpp
template <typename InputIterator, typename OutputIterator, typename T>
OutputIterator
__adjacent_difference(InputIterator first,InputIterator last,
                      OutputIterator result, T*)
{
    T value = *first;
    while(++first != last)
    {
        T tmp = *first;
        *++result = tmp - value;
        value = tmp;
    }
    return ++result;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator
adjacent_difference(InputIterator first, InputIterator last, OutputIterator result)
{
    if(first == last) return result;
    *result = *first;
    return __adjacent_difference(first, last, result, value_type(first));
}

template <typename InputIterator, typename OutputIterator, typename T,
         typename BinaryOperation>
OutputIterator
__adjacent_difference(InputIterator first, InputIterator last,
                      OutputIterator result, T*,
                      BinaryOperation binary_op)
{
    T value = *first;
    while(++first != last)
    {
        T tmp = *first;
        *++result = binary_op(tmp,value);
        value = tmp;
    }
    return ++result;
}

template <typename InputIterator, typename OutputIterator, typename BinaryOperation>
OutputIterator
adjacent_difference(InputIterator first, InputIterator last,
                    OutputIterator result, BinaryOperation binary_op)
{
    if(first == last) return result;
    *result = *first;
    return __adjacent_difference(first, last, result, value_type(first), binary_op);
}
```