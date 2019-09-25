heap并不归属与STL容器组件，它是个幕后英雄，扮演priority queue的助手。
heap在algorithm头文件中实现，sgi把它放在了stl_heap.h中。
共有4个操纵heap的函数，分别是：

	push_heap, pop_heap, make_heap, sort_heap
##堆的知识
###堆的定义
二叉堆是一种完全二叉树，它满足两个特性：
1．父结点的键值总是大于或等于（小于或等于）任何一个子节点的键值。
2．每个结点的左子树和右子树都是一个二叉堆（都是最大堆或最小堆）。
当父结点的键值总是大于或等于任何一个子节点的键值时为最大堆（max-heap）。当父结点的键值总是小于或等于任何一个子节点的键值时为最小堆（min-heap）。
###堆的存储
一般用数组来表示堆：
![存储](http://img.my.csdn.net/uploads/201108/22/0_1314014706gZqn.gif)
如果数组从0节点开始存储，i节点的父节点下标就为(i–1)/2，它的左右子节点下标分别为2*i+1和2*i+2。
如果数组的0节点弃用，从1节点开始存储，i节点的父节点下标为i/2，左右子节点下标分别为2*i和2*i+1。
###堆的push
新加入的元素放在数组的末尾处，然后执行一个上溯过程：将新节点与其父节点比较，如果比父节点大（对于max-heap），父子就兑换位置，如此一直上溯，直到不需要兑换或到根节点。
###堆的pop
取出根节点，将最后一个节点的值赋给根结点，然后再从根结点开始进行一次从上向下的调整。
调整时先在左右儿子结点中找最大的（对于max-heap），如果父结点比这个最大的子结点还大说明不需要调整了，反之将父结点和它交换后再考虑后面的结点。相当于从根结点开始将一个数据“下沉”的过程。
###堆化数组
最简单的一个想法是：一个一个元素的push进heap进行了。但实际上可以更高效一些，看上面的那张图，所有的叶子节点已经是合法的堆了，所以只要从节点56开始调整即可。
###参考
白话经典算法系列之七 堆与堆排序
http://blog.csdn.net/morewindows/article/details/6709644/
STL源码剖析，侯捷
##我的实现
根据以上思路写出的代码（可能用了两个小时）：
```cpp
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <memory>
using namespace std;

int buf[] = {34, 56, 43, 82};
vector<int> heap(buf, buf+4);

// 调整节点i, 使其符合max-heap的条件
// 假定节点i以后的节点都满足max-heap的条件
void adjust(int i)
{
    int j = 2*i+2; // 右子节点
    // 不断下沉
    while(j < heap.size()) // 右子节点存在
    {
        if(heap[j-1]>heap[j]) --j; // 找到子节点中较大的
        if(heap[j]<=heap[i]) break;
        swap(heap[j],heap[i]);
        i = j;
        j = 2*i+2;
    }
    if(j == heap.size()) // 只有左子节点, 没有右子节点的情况
    {
        if(heap[j-1]>heap[i]) swap(heap[j-1],heap[i]);
    }
    // i是叶子节点的情况不作处理
}

// 构建max-heap
void MakeHeap()
{
    int len = heap.size();
    // 共有(len+1)/2个叶子节点
    // 从 len-叶子节点数+1 开始构造堆, 即从(len+1)/2开始
    for(int i=(len+1)/2; i>=0; --i)
        adjust(i);
}

void PushHeap(int a)
{
    heap.push_back(a);
    int j = heap.size()-1; // 子节点
    // 上溯
    while(j > 0)
    {
        int i = (j-1)/2; // 父节点
        if(heap[j] <= heap[i]) break;
        swap(heap[j],heap[i]);
        j = i;
    }
}

void PopHeap()
{
    cout << *heap.begin() << ' ';
    *heap.begin() = *(heap.end()-1);
    heap.pop_back();
    adjust(0);
}

int main()
{
    MakeHeap();
    for(int i=0; i<10; ++i)
        PushHeap(rand()%99);
    for(int i=0; i<14; ++i)
        PopHeap();
    return 0;
}
```
##STL源码
下面是四个函数的声明：
```cpp
// 执行完毕后, [first,last)是一个合法的堆
template <class RandomAccessIterator>
void make_heap (RandomAccessIterator first, RandomAccessIterator last);
template <class RandomAccessIterator, class Compare>
void make_heap (RandomAccessIterator first, RandomAccessIterator last, Compare comp );

// [first,last-1)是一个合法的堆, last-1位置是新push的值, 执行完毕后[first,last)是新的堆
template <class RandomAccessIterator>
void push_heap (RandomAccessIterator first, RandomAccessIterator last);
template <class RandomAccessIterator, class Compare>
void push_heap (RandomAccessIterator first, RandomAccessIterator last, Compare comp);

// [first,last)是一个合法的堆, 执行完毕后[first,last-1)是新的堆, 原堆顶元素置于last-1位置
template <class RandomAccessIterator>
void pop_heap (RandomAccessIterator first, RandomAccessIterator last);
template <class RandomAccessIterator, class Compare>
void pop_heap (RandomAccessIterator first, RandomAccessIterator last, Compare comp);

// [first,last)是一个合法的堆, 执行完毕后原序列不再是堆, 而是有序的序列
template <class RandomAccessIterator>
void sort_heap (RandomAccessIterator first, RandomAccessIterator last);
template <class RandomAccessIterator, class Compare>
void sort_heap (RandomAccessIterator first, RandomAccessIterator last, Compare comp);
```
每个函数都有两个版本，第一个版本接受两个随机访问迭代器，第二个版本额外还接受一个比较对象。
以下是第一个版本的源码：
```cpp
//__first:序列起始位置 __holeIndex:hole的索引 __topIndex:堆顶索引 __value:要push的元素
template <class _RandomAccessIterator, class _Distance, class _Tp>
void
__push_heap(_RandomAccessIterator __first,
            _Distance __holeIndex, _Distance __topIndex, _Tp __value)
{
	// hole不断上浮, 小于__value的__parent不断下沉, 最后将__value放在合适的位置上
    _Distance __parent = (__holeIndex - 1) / 2;
    while (__holeIndex > __topIndex && *(__first + __parent) < __value)
    {
        *(__first + __holeIndex) = *(__first + __parent);
        __holeIndex = __parent;
        __parent = (__holeIndex - 1) / 2;
    }
    *(__first + __holeIndex) = __value;
}

template <class _RandomAccessIterator, class _Distance, class _Tp>
inline void
__push_heap_aux(_RandomAccessIterator __first,
                _RandomAccessIterator __last, _Distance*, _Tp*)
{
    __push_heap(__first, _Distance((__last - __first) - 1), _Distance(0),
                _Tp(*(__last - 1)));
}

template <class _RandomAccessIterator>
inline void
push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
    // 调用此函数时, 新元素应已置于容器的最尾端
    __push_heap_aux(__first, __last, __distance_type(__first), __value_type(__first));
}


template <class _RandomAccessIterator, class _Distance, class _Tp>
void
__adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex,
              _Distance __len, _Tp __value)
{
    _Distance __topIndex = __holeIndex;
    _Distance __secondChild = 2 * __holeIndex + 2;
    while (__secondChild < __len)
    {
        if (*(__first + __secondChild) < *(__first + (__secondChild - 1)))
            __secondChild--;
        *(__first + __holeIndex) = *(__first + __secondChild);
        __holeIndex = __secondChild;
        __secondChild = 2 * (__secondChild + 1);
    }
    if (__secondChild == __len)
    {
        *(__first + __holeIndex) = *(__first + (__secondChild - 1));
        __holeIndex = __secondChild - 1;
    }
    __push_heap(__first, __holeIndex, __topIndex, __value);
}

template <class _RandomAccessIterator, class _Tp, class _Distance>
inline void
__pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last,
           _RandomAccessIterator __result, _Tp __value, _Distance*)
{
    *__result = *__first;
    __adjust_heap(__first, _Distance(0), _Distance(__last - __first), __value);
}

template <class _RandomAccessIterator, class _Tp>
inline void
__pop_heap_aux(_RandomAccessIterator __first, _RandomAccessIterator __last,
               _Tp*)
{
    __pop_heap(__first, __last - 1, __last - 1,
               _Tp(*(__last - 1)), __distance_type(__first));
}

template <class _RandomAccessIterator>
inline void pop_heap(_RandomAccessIterator __first,
                     _RandomAccessIterator __last)
{
    __pop_heap_aux(__first, __last, __value_type(__first));
}


template <class _RandomAccessIterator, class _Tp, class _Distance>
void
__make_heap(_RandomAccessIterator __first,
            _RandomAccessIterator __last, _Tp*, _Distance*)
{
    if (__last - __first < 2) return;
    _Distance __len = __last - __first;
    _Distance __parent = (__len - 2)/2;

    while (true)
    {
        __adjust_heap(__first, __parent, __len, _Tp(*(__first + __parent)));
        if (__parent == 0) return;
        __parent--;
    }
}

template <class _RandomAccessIterator>
inline void
make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
    __make_heap(__first, __last,
                __value_type(__first), __distance_type(__first));
}


template <class _RandomAccessIterator>
void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
    while (__last - __first > 1)
        pop_heap(__first, __last--);
}
```
第二个版本与第一个版本只有比较的地方不同，其他地方完全一样，例如：
```cpp
    while (__holeIndex > __topIndex && *(__first + __parent) < __value)
    {
        *(__first + __holeIndex) = *(__first + __parent);
        __holeIndex = __parent;
        __parent = (__holeIndex - 1) / 2;
    }

    while (__holeIndex > __topIndex && __comp(*(__first + __parent), __value))
    {
        *(__first + __holeIndex) = *(__first + __parent);
        __holeIndex = __parent;
        __parent = (__holeIndex - 1) / 2;
    }
```
通过设置第二个版本的第三个模板参数，就可以实现min-heap。

总体而言，sgi的实现和我的实现大体相同，还有几点不同：
1. push和pop操作的元素的位置
调用push()之前，新元素应已置于容器的最尾端；
调用pop()之后，旧元素将置于容器的最尾端，所以，调用sort_heap()之后，两个迭代器之间的数据就有序了。
2. 交换动作
push()和pop()使用了hole的概念，通过hole的上浮和下沉来进行调整。