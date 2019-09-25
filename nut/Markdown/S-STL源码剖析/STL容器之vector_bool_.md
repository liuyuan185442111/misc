vector有一个针对bool类型的specialization，也定义于&lt;vector>中，sgi将它实现于&lt;stl_bvector.h>中：

	template < class T, class Alloc = allocator<T> > class vector; // generic template
	template <class Alloc> class vector<bool,Alloc>;               // bool specialization
这个东西和bitset的内部实现比较相似，vector&lt;bool>用unsigned int来存储比特位，大概思想就是以时间换空间。其实如果元素数量不大，用vector&lt;char>来表示bool数组也可以。

不过既然作为vector的一个特殊化，就要像标准容器靠拢，所以vector&lt;bool>支持insert()，push_back()，pop_back()，有迭代器。但它并不是一个标准的STL容器，vector&lt;bool>里的一个元素在unsigned long中只占一个比特，有指向bool的指针，但没有指向比特的指针，虽然vector&lt;bool>的迭代器表现已经很好了，但仍然不够好。

一个东西要成为STL容器就必须满足所有在C++标准23.1节中列出的容器必要条件。在这些要求中有这样一条：如果c是一个T类型对象的容器，且c支持operator[]，那么以下代码必须能够编译：

	T *p = &c[0]; // 无论operator[]返回什么
明显，vector&lt;bool>的迭代器不可能满足这个条件。
正因如此，很多人都建议避免使用vector&lt;bool>。可我觉得除了对迭代器解引用的结果不是bool类型，用起来也挺方便的，而且vector&lt;bool>的内部实现也可以借鉴。

只要设计好了迭代器，其他操作基本可以由vector照搬过来。
vector&lt;bool>的迭代器是随机访问迭代器，迭代器内部有两个成员变量来标识对应比特的位置：

	unsigned int* _M_p;
	unsigned int _M_offset;
迭代器的移动操作都（自加、自减、随机移动等）是在操作这两个变量，解引用和取下标操作却不同。

	reference operator*() const
	{ return reference(_M_p, 1U << _M_offset); }
	reference operator[](difference_type i)
	{ return *(*this + i); }
reference是一个代理，通过它可以对vector&lt;bool>的迭代器指向的元素赋值。它的原型至少应包含：
```cpp
class vector<bool>::reference {
  friend class vector;
  reference();                                  // no public constructor
public:
  ~reference();
  operator bool () const;                       // convert to bool
  reference& operator= (const bool x);          // assign from bool
  reference& operator= (const reference& x);    // assign from bit
  void flip();                                  // flip bit value
};
```
sgi的实现里，它内部也有两个数据成员来标记比特的位置：

	unsigned int* _M_p;	// 比特所在unsigned int的地址
	unsigned int _M_mask; // 比特在unsigned int中的位置
reference定义了bool强转运算符，所以可以bool b = a[i];
为bool定义了赋值运算符，所以可以a[i] = true;
还可以通过a[i].flip()来直接翻转比特位。

其他内容，参考vector和bitset实现。

拓展阅读
http://blog.csdn.net/suwei19870312/article/details/6610804
http://blog.csdn.net/liushu1231/article/details/8844631
http://www.cplusplus.com/reference/vector/vector-bool/