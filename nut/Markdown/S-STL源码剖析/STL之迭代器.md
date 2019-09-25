根据实现的功能，迭代器分为五类：
![迭代器分类](http://img.blog.csdn.net/20150519181138966)
不同种类迭代器可进行的操作（[from](http://www.cplusplus.com/reference/iterator/)）：
<table border="1"><tr><th colspan="4">category</th><th>properties</th><th>valid expressions</th></tr>
<tr><td colspan="4" rowspan="2">all categories</td><td>copy-constructible, copy-assignable and destructible</td><td>X b(a);<br>
b = a;</td></tr>
<tr><td>Can be incremented</td><td>++a<br>
a++</td></tr>
<tr><td rowspan="10">Random Access</td><td rowspan="6">Bidirectional</td><td rowspan="5">Forward</td><td rowspan="2">Input</td><td>Supports equality/inequality comparisons</td><td>a == b<br>
a != b</td></tr>
<tr><td>Can be dereferenced as an rvalue</td><td>*a<br>
a-&gt;m</td></tr>
<tr><td>Output</td><td>Can be dereferenced as an lvalue<br>
(only for mutable iterator types)</td><td>*a = t<br>
*a++ = t</td></tr>
<tr><td rowspan="2"></td><td>default-constructible</td><td>X a;<br>
X()</td></tr>
<tr><td>Multi-pass: neither dereferencing nor<br>
incrementing affects dereferenceability</td><td>{ b=a; *a++; *b; }</td></tr>
<tr><td colspan="2"></td><td>Can be decremented</td><td><xcode>--a<br>
a--<br>
*a--</xcode></td></tr>
<tr><td colspan="3" rowspan="4"></td><td>Supports arithmetic operators + and -</td><td>a + n<br>
n + a<br>
a - n<br>
a - b</td></tr>
<tr><td>Supports inequality comparisons (&lt;, &gt;, &lt;= and &gt;=)<br>between iterators</td><td>a &lt; b<br>
a &gt; b<br>
a &lt;= b<br>
a &gt;= b</td></tr>
<tr><td>Supports compound assignment operations += and -=</td><td>a += n<br>
a -= n</td></tr>
<tr><td>Supports offset dereference operator ([])</td><td>a[n]</td></tr>
</table>
##迭代器的结构
源码在stl_iterator_base_types.h中。
```cpp
// 5种迭代器类别
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

// 迭代器基类, 自行开发的迭代器最好继承此基类
template <typename Category, typename T, typename Distance = ptrdiff_t, typename Pointer = T*, typename Reference = T&>
struct iterator
{
    // Category为上面5种类别之一
    typedef Category  iterator_category;
    typedef T         value_type;
    typedef Distance  difference_type;
    typedef Pointer   pointer;
    typedef Reference reference;
};

// 提取出迭代器(模板参数)的内置类型
template <typename Iterator>
struct iterator_traits
{
    typedef typename Iterator::iterator_category iterator_category;
    typedef typename Iterator::value_type        value_type;
    typedef typename Iterator::difference_type  difference_type;
    typedef typename Iterator::pointer            pointer;
    typedef typename Iterator::reference         reference;
};

// 常规指针的偏特化版本
template <typename T>
struct iterator_traits<T*>
{
    typedef random_access_iterator_tag iterator_category;
    typedef T                         value_type;
    typedef ptrdiff_t                 difference_type;
    typedef T*                        pointer;
    typedef T&                        reference;
};

// 常规const指针的偏特化版本
template <class T>
struct iterator_traits<const T*>
{
    typedef random_access_iterator_tag iterator_category;
    typedef T                         value_type;
    typedef ptrdiff_t                 difference_type;
    typedef const T*                  pointer;
    typedef const T&                  reference;
};


// These functions is not a part of the C++ standard but is syntactic sugar for internal library use only
template<typename Iter>
inline typename iterator_traits<Iter>::iterator_category
__iterator_category(const Iter&)
{
    return typename iterator_traits<Iter>::iterator_category();
}

template <class Iter>
inline typename iterator_traits<Iter>::difference_type*
__distance_type(const Iter&)
{
	return static_cast<typename iterator_traits<Iter>::difference_type*>(0);
}

template <class Iter>
inline typename iterator_traits<Iter>::value_type*
__value_type(const Iter&)
{
	return static_cast<typename iterator_traits<Iter>::value_type*>(0);
}

template <class Iter>
inline typename iterator_traits<Iter>::iterator_category
iterator_category(const Iter& i) { return __iterator_category(i); }

template <class Iter>
inline typename iterator_traits<Iter>::difference_type*
distance_type(const Iter& i) { return __distance_type(i); }

template <class Iter>
inline typename iterator_traits<Iter>::value_type*
value_type(const Iter& i) { return __value_type(i); }
```
##几个基本的函数
源码在stl_iterator_base_funcs.h中。
```cpp
// 计算两个迭代器之间的距离
template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, input_iterator_tag)
{
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last)
    {
        ++first;
        ++n;
    }
    return n;
}

template <typename RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
{
    return last - first;
}

template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
    typedef typename iterator_traits<InputIterator>::iterator_category Category;
    return __distance(first, last, Category());
}

// 前进
template <typename InputIterator, typename Distance>
inline void __advance(InputIterator& i, Distance n, input_iterator_tag)
{
    while (n--) ++i;
}

template <typename BidirectionalIterator, typename Distance>
inline void __advance(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
{
    if (n > 0)
        while (n--) ++i;
    else
        while (n++) --i;
}

template <typename RandomAccessIterator, typename Distance>
inline void __advance(RandomAccessIterator& i, Distance n, random_access_iterator_tag)
{
    i += n;
}

template <typename InputIterator, typename Distance>
inline void advance(InputIterator& i, Distance n)
{
    typedef typename iterator_traits<InputIterator>::iterator_category Category;
    __advance(i, n, Category());
}
```
##迭代器适配器
有3种，分别为插入迭代器，反向迭代器，输入输出迭代器。
###插入迭代器
做的工作是：将赋值操作改为插入操作。分为back_insert_iterator，front_insert_iterator，insert_iterator三种。insert_iterator不仅进行插入操作，还会进行一个右移操作，这样就可以进行持续的插入。
为了操作方便，还定义了3个函数，他们返回相应的插入迭代器。
```cpp
template <typename Container>
class back_insert_iterator : public iterator<output_iterator_tag,void,void,void,void>
{
protected:
    Container* container;
public:
    typedef Container container_type;

    explicit back_insert_iterator(Container& x) : container(&x) {}
    back_insert_iterator<Container>&
    operator=(const typename Container::value_type& value)
    {
        container->push_back(value);
        return *this;
    }
    back_insert_iterator<Container>& operator*()
    { return *this; }
    back_insert_iterator<Container>& operator++()
    { return *this; }
    back_insert_iterator<Container>& operator++(int)
    { return *this; }
};

template <typename Container>
inline back_insert_iterator<Container>
back_inserter(Container& x)
{
    return back_insert_iterator<Container>(x);
}

template <typename Container>
class front_insert_iterator : public iterator<output_iterator_tag,void,void,void,void>
{
protected:
    Container* container;
public:
    typedef Container container_type;

    explicit front_insert_iterator(Container& x) : container(&x) {}
    front_insert_iterator<Container>&
    operator=(const typename Container::value_type& value)
    {
        container->push_front(value);
        return *this;
    }
    front_insert_iterator<Container>& operator*()
    { return *this; }
    front_insert_iterator<Container>& operator++()
    { return *this; }
    front_insert_iterator<Container>& operator++(int)
    { return *this; }
};

template <typename Container>
inline front_insert_iterator<Container>
front_inserter(Container& x)
{
    return front_insert_iterator<Container>(x);
}

template <class Container>
class insert_iterator : public iterator<output_iterator_tag,void,void,void,void>
{
protected:
    Container* container;
    typename Container::iterator iter;
public:
    typedef Container container_type;

    insert_iterator(Container& x, typename Container::iterator i)
        : container(&x), iter(i) {}
    insert_iterator<Container>&
    operator=(const typename Container::value_type& value)
    {
        iter = container->insert(iter, value);
        ++iter;
        return *this;
    }
    insert_iterator<Container>& operator*()
    { return *this; }
    insert_iterator<Container>& operator++()
    { return *this; }
    insert_iterator<Container>& operator++(int)
    { return *this; }
};

template <typename Container, typename Iterator>
inline insert_iterator<Container>
inserter(Container& x, Iterator i)
{
    typedef typename Container::iterator iter;
    return insert_iterator<Container>(x, iter(i));
}
```
###反向迭代器
反向迭代器，顾名思义。
```cpp
template <typename Iterator>
class reverse_iterator
{
protected:
    Iterator current;
public:
    typedef typename iterator_traits<Iterator>::iterator_category
    iterator_category;
    typedef typename iterator_traits<Iterator>::value_type
    value_type;
    typedef typename iterator_traits<Iterator>::difference_type
    difference_type;
    typedef typename iterator_traits<Iterator>::pointer
    pointer;
    typedef typename iterator_traits<Iterator>::reference
    reference;

    typedef Iterator iterator_type;
    typedef reverse_iterator<Iterator> Self;

public:
    reverse_iterator() {}
    explicit reverse_iterator(iterator_type x) : current(x) {}

    reverse_iterator(const Self& x) : current(x.current) {}
    template <class Iter>
    reverse_iterator(const reverse_iterator<Iter>& x)
        : current(x.current) {}

    iterator_type base() const
    {
        return current;
    }
    reference operator*() const
    {
        Iterator tmp = current;
        return *--tmp;
    }
    pointer operator->() const
    {
        return &(operator*());
    }

    Self& operator++()
    {
        --current;
        return *this;
    }
    Self operator++(int)
    {
        Self tmp = *this;
        --current;
        return tmp;
    }
    Self& operator--()
    {
        ++current;
        return *this;
    }
    Self operator--(int)
    {
        Self tmp = *this;
        ++current;
        return tmp;
    }

    Self operator+(difference_type n) const
    {
        return Self(current - n);
    }
    Self& operator+=(difference_type n)
    {
        current -= n;
        return *this;
    }
    Self operator-(difference_type n) const
    {
        return _Self(current + n);
    }
    Self& operator-=(difference_type n)
    {
        current += n;
        return *this;
    }
    reference operator[](difference_type n) const
    {
        return *(*this + n);
    }
};

template <typename Iterator>
inline bool operator==(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
    return x.base() == y.base();
}

template <typename Iterator>
inline bool operator<(const reverse_iterator<Iterator>& x,
                      const reverse_iterator<Iterator>& y)
{
    return y.base() < x.base();
}

template <typename Iterator>
inline bool operator!=(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
    return !(x == y);
}

template <typename Iterator>
inline bool operator>(const reverse_iterator<Iterator>& x,
                      const reverse_iterator<Iterator>& y)
{
    return y < x;
}

template <typename Iterator>
inline bool operator<=(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
    return !(y < x);
}

template <typename Iterator>
inline bool operator>=(const reverse_iterator<Iterator>& x,
                       const reverse_iterator<Iterator>& y)
{
    return !(x < y);
}

template <typename Iterator>
inline typename reverse_iterator<Iterator>::difference_type
operator-(const reverse_iterator<Iterator>& x,
          const reverse_iterator<Iterator>& y)
{
    return y.base() - x.base();
}

template <typename Iterator>
inline reverse_iterator<Iterator>
operator+(typename reverse_iterator<Iterator>::difference_type n,
          const reverse_iterator<Iterator>& x)
{
    return x.operator+(n);
}
```
###输入输出迭代器
输入迭代器有两个，分别为istream_iterator和istreambuf_iterator；输出迭代器有两个，分别为ostream_iterator和ostreambuf_iterator。
输入迭代器将自加操作转换为输入操作，输出迭代器将赋值转换为输出。
实现也不难，直接看源码吧。
```cpp
template <class _Tp,
         class _CharT = char, class _Traits = char_traits<_CharT>,
         class _Dist = ptrdiff_t>
class istream_iterator
{
public:
    typedef _CharT                         char_type;
    typedef _Traits                        traits_type;
    typedef basic_istream<_CharT, _Traits> istream_type;

    typedef input_iterator_tag             iterator_category;
    typedef _Tp                            value_type;
    typedef _Dist                          difference_type;
    typedef const _Tp*                     pointer;
    typedef const _Tp&                     reference;

    istream_iterator() : _M_stream(0), _M_ok(false) {}
    istream_iterator(istream_type& s) : _M_stream(&s)
    {
        _M_read();
    }

    reference operator*() const
    {
        return _M_value;
    }
    pointer operator->() const
    {
        return &(operator*());
    }

    istream_iterator& operator++()
    {
        _M_read();
        return *this;
    }
    istream_iterator operator++(int)
    {
        istream_iterator tmp = *this;
        _M_read();
        return tmp;
    }

    bool _M_equal(const istream_iterator& x) const
    {
        return (_M_ok == x._M_ok) && (!_M_ok || _M_stream == x._M_stream);
    }

private:
    istream_type* _M_stream;
    _Tp _M_value;
    bool _M_ok;

    void _M_read()
    {
        _M_ok = (_M_stream && *_M_stream) ? true : false;
        if (_M_ok)
        {
            *_M_stream >> _M_value;
            _M_ok = *_M_stream ? true : false;
        }
    }
};

template <class _Tp, class _CharT, class _Traits, class _Dist>
inline bool
operator==(const istream_iterator<_Tp, _CharT, _Traits, _Dist>& x,
           const istream_iterator<_Tp, _CharT, _Traits, _Dist>& y)
{
    return x._M_equal(y);
}

template <class _Tp, class _CharT, class _Traits, class _Dist>
inline bool
operator!=(const istream_iterator<_Tp, _CharT, _Traits, _Dist>& x,
           const istream_iterator<_Tp, _CharT, _Traits, _Dist>& y)
{
    return !x._M_equal(y);
}


template <class _Tp, class _CharT = char, class _Traits = char_traits<_CharT> >
class ostream_iterator
{
public:
    typedef _CharT                         char_type;
    typedef _Traits                        traits_type;
    typedef basic_ostream<_CharT, _Traits> ostream_type;

    typedef output_iterator_tag            iterator_category;
    typedef void                           value_type;
    typedef void                           difference_type;
    typedef void                           pointer;
    typedef void                           reference;

    ostream_iterator(ostream_type& s) : _M_stream(&s), _M_string(0) {}
    ostream_iterator(ostream_type& s, const _CharT* c)
        : _M_stream(&s), _M_string(c)  {}
    ostream_iterator<_Tp>& operator=(const _Tp& value)
    {
        *_M_stream << value;
        if (_M_string) *_M_stream << _M_string;
        return *this;
    }
    ostream_iterator<_Tp>& operator*()
    {
        return *this;
    }
    ostream_iterator<_Tp>& operator++()
    {
        return *this;
    }
    ostream_iterator<_Tp>& operator++(int)
    {
        return *this;
    }
private:
    ostream_type* _M_stream;
    const _CharT* _M_string;
};
```
ostreambuf_iterator和istreambuf_iterator的源码在sbuf_iter.h文件中，我也没有细看，直接贴上来了。
```cpp
template<typename _CharT, typename _Traits>
class ostreambuf_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
{
public:
    typedef _CharT                           char_type;
    typedef _Traits                          traits_type;
    typedef basic_streambuf<_CharT, _Traits> streambuf_type;
    typedef basic_ostream<_CharT, _Traits>   ostream_type;

private:
    streambuf_type* 	_M_sbuf;
    bool 		_M_failed;

public:
    inline ostreambuf_iterator(ostream_type& __s) throw ()
        : _M_sbuf(__s.rdbuf()), _M_failed(!_M_sbuf) { }

    ostreambuf_iterator(streambuf_type* __s) throw ()
        : _M_sbuf(__s), _M_failed(!_M_sbuf) { }

    ostreambuf_iterator&
    operator=(_CharT __c);

    ostreambuf_iterator&
    operator*() throw()
    {
        return *this;
    }

    ostreambuf_iterator&
    operator++(int) throw()
    {
        return *this;
    }

    ostreambuf_iterator&
    operator++() throw()
    {
        return *this;
    }

    bool
    failed() const throw()
    {
        return _M_failed;
    }
};

template<typename _CharT, typename _Traits>
inline ostreambuf_iterator<_CharT, _Traits>&
ostreambuf_iterator<_CharT, _Traits>::operator=(_CharT __c)
{
    if (!_M_failed &&
            _Traits::eq_int_type(_M_sbuf->sputc(__c),_Traits::eof()))
        _M_failed = true;
    return *this;
}


template<typename _CharT, typename _Traits>
class istreambuf_iterator
    : public iterator<input_iterator_tag, _CharT, typename _Traits::off_type,
      _CharT*, _CharT&>
{
public:
    typedef _CharT                         		char_type;
    typedef _Traits                        		traits_type;
    typedef typename _Traits::int_type     		int_type;
    typedef basic_streambuf<_CharT, _Traits> 		streambuf_type;
    typedef basic_istream<_CharT, _Traits>         	istream_type;
    typedef istreambuf_iterator<_CharT, _Traits>	__istreambufiter_type;

private:
    // If the end of stream is reached (streambuf_type::sgetc()
    // returns traits_type::eof()), the iterator becomes equal to
    // the "end of stream" iterator value.
    // NB: This implementation assumes the "end of stream" value
    // is EOF, or -1.
    streambuf_type* 		_M_sbuf;
    int_type 			_M_c;

public:
    istreambuf_iterator() throw()
        : _M_sbuf(NULL), _M_c(-2) { }

    istreambuf_iterator(istream_type& __s) throw()
        : _M_sbuf(__s.rdbuf()), _M_c(-2) { }

    istreambuf_iterator(streambuf_type* __s) throw()
        : _M_sbuf(__s), _M_c(-2) { }

    // NB: This should really have an int_type return
    // value, so "end of stream" postion can be checked without
    // hacking.
    char_type
    operator*() const
    {
        // The result of operator*() on an end of stream is undefined.
        char_type __ret;
        if (_M_sbuf && _M_c != static_cast<int_type>(-2))
            __ret = _M_c;
        else if (_M_sbuf)
            __ret = traits_type::to_char_type(_M_sbuf->sgetc());
        else
            __ret = static_cast<char_type>(traits_type::eof());
        return __ret;
    }

    __istreambufiter_type&
    operator++()
    {
        if (_M_sbuf)
            _M_sbuf->sbumpc();
        _M_c = -2;
        return *this;
    }

    __istreambufiter_type
    operator++(int)
    {
        __istreambufiter_type __old = *this;
        if (_M_sbuf)
            __old._M_c = _M_sbuf->sbumpc();
        _M_c = -2;
        return __old;
    }

    bool
    equal(const __istreambufiter_type& __b)
    {
        int_type __eof = traits_type::eof();
        bool __thiseof = !_M_sbuf || _M_sbuf->sgetc() == __eof;
        bool __beof = !__b._M_sbuf
                      || __b._M_sbuf->sgetc() == __eof;
        return (__thiseof && __beof || (!__thiseof && !__beof));
    }
};

template<typename _CharT, typename _Traits>
inline bool
operator==(const istreambuf_iterator<_CharT, _Traits>& __a,
           const istreambuf_iterator<_CharT, _Traits>& __b)
{
    return __a.equal(__b);
}

template<typename _CharT, typename _Traits>
inline bool
operator!=(const istreambuf_iterator<_CharT, _Traits>& __a,
           const istreambuf_iterator<_CharT, _Traits>& __b)
{
    return !__a.equal(__b);
}
```