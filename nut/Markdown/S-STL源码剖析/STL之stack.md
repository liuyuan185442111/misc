栈，LIFO的数据结构。

	template <typename T, typename Container = deque<T> > class stack;
stack是一个容器适配器，数据从容器的一端插入和取出（称为top端）。内部有一个Container类型的private变量，用来存储数据。

容器只要支持如下操作即可作为stack的内部容器：

	empty()、size()、back()、push_back()、pop_back()
vector，deque，list都满足要求，默认使用的是deque。

根据标准，stack有3个成员类型：

	value_type		第一个模板参数		栈元素的类型
	container_type	第二个模板参数		栈内部容器的类型
	size_type		一个无符号整型		通常是size_t类型

stack还有一些成员函数，它们都是public的：

	explicit stack (const container_type& ctnr = container_type());
	参数类型须和第二个模板参数相同

	bool empty() const;
	判断stack是否为空，即size是否为0，调用内部容器的empty()

	size_type size() const;
	调用内部容器的size()

	value_type& top();
	const value_type& top() const;
	返回顶部元素，调用内部容器的back()

	void push (const value_type& val);
	插入一个元素，被初始化为val的拷贝，调用内部容器的push_back()

	void pop();
	移除顶部元素，会调用移除元素的析构函数，调用内部容器的pop_back()

此外，stack还有一些关系操作符，它们都是模板函数。（lhs=the left-hand side）

	template <class T, class Container>
	bool operator== (const stack<T,Container>& lhs, const stack<T,Container>& rhs);
	template <class T, class Container>
	bool operator!= (const stack<T,Container>& lhs, const stack<T,Container>& rhs);
	template <class T, class Container>
	bool operator<  (const stack<T,Container>& lhs, const stack<T,Container>& rhs);
	template <class T, class Container>
	bool operator<= (const stack<T,Container>& lhs, const stack<T,Container>& rhs);
	template <class T, class Container>
	bool operator>  (const stack<T,Container>& lhs, const stack<T,Container>& rhs);
	template <class T, class Container>
	bool operator>= (const stack<T,Container>& lhs, const stack<T,Container>& rhs);

下面是stack的完整源代码（stl_stack.h）：
```cpp
// operator==和operator<的前向声明对于友元的声明是必须的
// stack的前向声明对于operator==和operator<的声明是必须的
template <typename _Tp, typename _Sequence = deque<_Tp> >
class stack;

template <typename _Tp, typename _Seq>
bool operator==(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y);
template <typename _Tp, typename _Seq>
bool operator<(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y);

template <typename _Tp, typename _Sequence>
class stack
{
    template <typename _Tp1, typename _Seq1>
    friend bool operator== (const stack<_Tp1, _Seq1>&, const stack<_Tp1, _Seq1>&);
    template <typename _Tp1, typename _Seq1>
    friend bool operator< (const stack<_Tp1, _Seq1>&, const stack<_Tp1, _Seq1>&);

public:
    typedef typename _Sequence::value_type      value_type;
    typedef typename _Sequence::size_type       size_type;
    typedef           _Sequence                  container_type;

    typedef typename _Sequence::reference       reference;
    typedef typename _Sequence::const_reference const_reference;

protected:
    _Sequence c;
    
public:
    stack() : c() {}
    explicit stack(const _Sequence& __s) : c(__s) {}
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference top() { return c.back(); }
    const_reference top() const { return c.back(); }
    void push(const value_type& __x) { c.push_back(__x); }
    void pop() { c.pop_back(); }
};

template <typename _Tp, typename _Seq>
bool operator==(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y)
{
    return __x.c == __y.c;
}

template <typename _Tp, typename _Seq>
bool operator<(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y)
{
    return __x.c < __y.c;
}

template <typename _Tp, typename _Seq>
bool operator!=(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y)
{
    return !(__x == __y);
}

template <typename _Tp, typename _Seq>
bool operator>(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y)
{
    return __y < __x;
}

template <typename _Tp, typename _Seq>
bool operator<=(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y)
{
    return !(__y < __x);
}

template <typename _Tp, typename _Seq>
bool operator>=(const stack<_Tp,_Seq>& __x, const stack<_Tp,_Seq>& __y)
{
    return !(__x < __y);
}
```