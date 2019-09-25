	template < typename T, typename Alloc = allocator<T> >
	class vector;

《源码剖析》上vector部分倒也不复杂，可源码也不是太简单，我大体浏览了一遍，发现如真要详细看看，也需要不少时间。所以就理解了个大概，空间重新配置的代码就不再赘述。

vector部分源码（摘录于stl_vector.h，有改动）：
```cpp
template < typename T, typename Alloc = allocator<T> >
class vector
{
public:
	typedef T value_type;
	typedef value_type* iterator;
	typedef const value_type* const_iterator;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef Alloc allocator_type;
	typedef reverse_iterator<const_iterator> const_reverse_iterator;
	typedef reverse_iterator<iterator> reverse_iterator;

protected:
	iterator start;
	iterator finish;
	iterator end_of_storage;

public:
	// 构造函数
	explicit vector(const allocator_type& a = allocator_type());
	explicit vector(size_type n, const T& value = value_type(),
					const allocator_type& a = allocator_type());
	vector(const vector& x);
	template <class InputIterator>
	vector(InputIterator first, InputIterator last,
		   const allocator_type& a = allocator_type());
	// 析构函数, 将所有元素析构
	~vector()
	{
		destroy(start, finish);
	}
	// 赋值运算符
	vector& operator=(const vector& x);
	// 获得空间配置器
	allocator_type get_allocator() const;

///////////////////////////////////////////////////////
	// 迭代器
	iterator begin()
	{
		return iterator(start);
	}
	const_iterator begin() const
	{
		return const_iterator(start);
	}
	iterator end()
	{
		return iterator(finish);
	}
	const_iterator end() const
	{
		return const_iterator(finish);
	}
	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}
	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(end());
	}
	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}
	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(begin());
	}

///////////////////////////////////////////////////////
	// 容量
	size_type size() const
	{
		return size_type(end() - begin());
	}
	size_type max_size() const
	{
		return size_type(-1) / sizeof(T);
	}
	size_type capacity() const
	{
		return size_type(iterator(end_of_storage) - begin());
	}
	bool empty() const
	{
		return begin() == end();
	}
	// 请求vector可容纳n个元素, 如容量不足, 扩展至n
	void reserve(size_type n);
	// resize使vector可容纳new_size个元素,
	// 如new_size不大于size(), 将new_size后面的元素析构,
	// 如new_size大于size(), 将元素个数扩展至new_size个, 可能会进行空间的扩展
	void resize(size_type new_size, const T& x = T());

///////////////////////////////////////////////////////
	// 元素访问
	reference operator[](size_type n)
	{
		return *(begin() + n);
	}
	const_reference operator[](size_type n) const
	{
		return *(begin() + n);
	}
	reference at(size_type n)
	{
		if (n >= this->size())
			__throw_out_of_range("vector");
		return (*this)[n];
	}
	const_reference at(size_type n) const
	{
		if (n >= this->size())
			__throw_out_of_range("vector");
		return (*this)[n];
	}
	reference front()
	{
		return *begin();
	}
	const_reference front() const
	{
		return *begin();
	}
	reference back()
	{
		return *(end() - 1);
	}
	const_reference back() const
	{
		return *(end() - 1);
	}

///////////////////////////////////////////////////////
	// Modifiers
	// 赋值操作, 可能会进行空间的扩展
	void assign(size_type n, const T& __val);
	template <class InputIterator>
	void assign(InputIterator first, InputIterator last);
	void push_back(const T& x);
	void pop_back()
	{
		--finish;
		destroy(finish);
	}
	template <class InputIterator>
	void insert(iterator pos, InputIterator first, InputIterator last);
	void insert (iterator pos, size_type n, const T& x);
	// 擦除, 会调用析构
	iterator erase(iterator position)
	{
		if (position + 1 != end())
			copy(position + 1, end(), position);
		--finish;
		_Destroy(finish);
		return position;
	}
	iterator erase(iterator first, iterator last)
	{
		iterator i(copy(last, end(), first));
		_Destroy(i, end());
		finish = finish - (last - first);
		return first;
	}
	// 擦除所有元素
	void clear()
	{
		erase(begin(), end());
	}
	void swap(vector<T, Alloc>& x)
	{
		std::swap(start, x.start);
		std::swap(finish, x.finish);
		std::swap(end_of_storage, x.end_of_storage);
	}
};

/**
* 比较运算符和swap函数
* equal, STL泛型算法函数, 用于按元素比较两个序列
* lexicographical_compare, STL泛型算法函数, 用于按字典序比较两个序列
*/
template <class T, class Alloc>
inline bool
operator==(const vector<T, Alloc>& x, const vector<T, Alloc>& y)
{
	return x.size() == y.size() && equal(x.begin(), x.end(), y.begin());
}

template <class T, class Alloc>
inline bool
operator<(const vector<T, Alloc>& x, const vector<T, Alloc>& y)
{
	return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

template <class T, class Alloc>
inline void swap(vector<T, Alloc>& x, vector<T, Alloc>& y)
{
	x.swap(y);
}

template <class T, class Alloc>
inline bool
operator!=(const vector<T, Alloc>& x, const vector<T, Alloc>& y)
{
	return !(x == y);
}

template <class T, class Alloc>
inline bool
operator>(const vector<T, Alloc>& x, const vector<T, Alloc>& y)
{
	return y < x;
}

template <class T, class Alloc>
inline bool
operator<=(const vector<T, Alloc>& x, const vector<T, Alloc>& y)
{
	return !(y < x);
}

template <class T, class Alloc>
inline bool
operator>=(const vector<T, Alloc>& x, const vector<T, Alloc>& y)
{
	return !(x < y);
}
```
##精要
vector的内部数据结构就是一段连续的内存，也就是一个数组，和string极像。有3个数据成员指示这段内存的状态，start，finish，end_of_storage。它们都是vector::iterator类型的，这个实现里vector::iterator就是T*类型，vector的迭代器就是普通指针，其他库的实现可能并不是这样。这三个数据成员可以完全标识一个vector的状态：start表示目前使用空间的头，finish表示目前使用空间的尾，end_of_storage表示目前可用空间的尾。

构造函数有一个参数是空间配置器对象，所以vector内部应有一个地方存这个对象，并用它进行内存配置操作。get_allocator()返回的也应是此对象。

增大空间包括3个步骤：配置新空间，数据 移动，释还旧空间。成本较高，所以配置的内存空间只增不减，即使用resize()来操作。
执行push操作时，空间增长策略是，如果当前空间为0，则增长为1，否则，增长一倍。
assign，reserve引起的空间增长，直接增大至请求的大小。
resize，insert引起的空间增长，假设需要增大n，空间增至end() - begin() + n和2*(end() - begin())中较大者。
<font color=red>**一旦引起空间重新配置，指向原vector的所有迭代器都将失效。**</font>

正因为重新配置空间成本较高，所以在创建vector时就确定其大小会提升效率，恰当时候使用reserve()也会提高效率。
##erase时的析构问题
注意到两个erase操作会调用_Destroy函数，_Destroy在[STL之处理uninitialized memory](http://blog.csdn.net/liuyuan185442111/article/details/45851157)中有介绍。

	iterator erase(iterator position)
	{
		if (position + 1 != end())
			copy(position + 1, end(), position);
		--finish;
		_Destroy(finish);
		return position;
	}
	iterator erase(iterator first, iterator last)
	{
		iterator i(copy(last, end(), first));
		_Destroy(i, end());
		finish = finish - (last - first);
		return first;
	}
问题在于，调用析构的元素并不是擦除掉的元素，而是末尾的元素！
特意写了一个例子验证：
```cpp
#include <vector>
#include <iostream>
using namespace std;

struct element
{
        int value;
        element(int i):value(i){ cout << "construct " << i << endl; }
        element(const element &val)
        {
                cout << "copy " << val.value << endl;
                value = val.value;
        }
        ~element() { cout << "destruct " << value << endl; }
};

int main()
{
        vector<element> v;
        v.reserve(4);
        cout << "push_back:\n";
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        cout << "\nerase:\n";
        v.erase(v.begin()+1);
        cout << "\nend:\n";
        return 0;
}
```
Windows和Linux下的gcc输出结果都是：
push_back:
construct 1
copy 1
destruct 1
construct 2
copy 2
destruct 2
construct 3
copy 3
destruct 3
construct 4
copy 4
destruct 4

erase:
destruct 4

end:
destruct 1
destruct 3
destruct 4
erase擦除的是第2个元素，被析构的却是最后一个元素。
**真是令人诧异**
根据[《stl:vector erase 时的元素析构问题》](http://www.voidcn.com/article/p-kejrmdzo-um.html)，这是一个已知的bug，并且是符合c++标准的。目瞪口呆.jpg
