头文件queue里定义了queue和priority_queue，它们和stack一样，都是适配器。
##queue
queue和stack的实现方式几乎相同，主要实现代码（stl_queue.h）：
```cpp
bool empty() const { return c.empty(); }
size_type size() const { return c.size(); }
reference front() { return c.front(); }
const_reference front() const { return c.front(); }
reference back() { return c.back(); }
const_reference back() const { return c.back(); }
void push(const value_type& __x) { c.push_back(__x); }
void pop() { c.pop_front(); }
```
由上面代码得知，内部容器需要有下面的方法：

	empty()、size()、front()、back()、push_back()、pop_front()
deque和list都满足这些要求，默认使用的是deque。
##priority_queue
优先队列是一个拥有权值观念的queue，由于这是一个queue，所以只允许在底端加入元素，并从顶端取出元素。
priority_queue带有权值观念，其内的元素并非依照被推入的次序排列，而是自动依照元素的权值排列（通常权值以实值表示）。权值最高者，排在最前面。

缺省情况下priority_queue用一个max-heap完成，后者是一个以vector表现的完全二叉树。
priority_queue本身的实现不难，重点在于heap的构建，我将另写一篇文章来介绍heap。

下面是priority_queue的源码（stl_queue.h）：
```cpp
template <typename _Tp, typename _Sequence = vector<_Tp>,
         typename _Compare = less<typename _Sequence::value_type> >
class priority_queue
{
public:
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef          _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;
    
protected:
    _Sequence c;
    _Compare comp; // 元素大小比较标准
    
public:
    explicit priority_queue(const _Compare& __x = _Compare(),
                            const _Sequence& __s = _Sequence())
        : c(__s), comp(__x)
    {
        make_heap(c.begin(), c.end(), comp);
    }

    template <class _InputIterator>
    priority_queue(_InputIterator __first, _InputIterator __last,
                   const _Compare& __x = _Compare(),
                   const _Sequence& __s = _Sequence())
        : c(__s), comp(__x)
    {
        c.insert(c.end(), __first, __last);
        make_heap(c.begin(), c.end(), comp);
    }

    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    const_reference top() const { return c.front(); }
    void push(const value_type& __x)
    {
        try
        {
            c.push_back(__x);
            push_heap(c.begin(), c.end(), comp);
        }
        catch(...) 
        {
            c.clear();
            throw;
        }
    }
    void pop()
    {
        try
        {
            pop_heap(c.begin(), c.end(), comp);
            c.pop_back();
        }
        catch(...)
        {
            c.clear();
            throw;
        }
    }
};
```
然后看一个简单的例子：
```cpp
// priority_queue::push/pop
#include <iostream>       // std::cout
#include <queue>          // std::priority_queue

int main ()
{
  std::priority_queue<int> mypq;

  mypq.push(30);
  mypq.push(100);
  mypq.push(25);
  mypq.push(40);

  std::cout << "Popping out elements...";
  while (!mypq.empty())
  {
     std::cout << ' ' << mypq.top();
     mypq.pop();
  }
  std::cout << '\n';

  return 0;
}
```
输出结果是：

	Popping out elements... 100 40 30 25