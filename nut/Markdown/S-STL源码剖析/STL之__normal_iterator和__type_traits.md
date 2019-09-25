__normal_iterator和__type_traits都是模板类，它们不属于c++标准里，而是sgi自己扩展的。
##__normal_iterator
__normal_iterator的主要目的是将指针, 转化为类型是class的迭代器。做的事情也就是把指针的各种操作封装为函数。basic_string中就使用了它：

	typedef __normal_iterator<pointer, basic_string>        iterator;
	typedef __normal_iterator<const_pointer, basic_string>  const_iterator;
源代码在stl_iterator.h中：
```cpp
template <typename _Iterator, typename _Container>
class __normal_iterator
    : public iterator<iterator_traits<_Iterator>::iterator_category,
      iterator_traits<_Iterator>::value_type,
      iterator_traits<_Iterator>::difference_type,
      iterator_traits<_Iterator>::pointer,
      iterator_traits<_Iterator>::reference>
{
protected:
    _Iterator _M_current;

public:
    typedef __normal_iterator<_Iterator, _Container> normal_iterator_type;
    typedef iterator_traits<_Iterator>  			__traits_type;
    typedef typename __traits_type::iterator_category 	iterator_category;
    typedef typename __traits_type::value_type 		value_type;
    typedef typename __traits_type::difference_type 	difference_type;
    typedef typename __traits_type::pointer          	pointer;
    typedef typename __traits_type::reference 		reference;

    __normal_iterator() : _M_current(_Iterator()) { }

    explicit __normal_iterator(const _Iterator& __i) : _M_current(__i) { }

    // Allow iterator to const_iterator conversion
    template<typename _Iter>
    inline __normal_iterator(const __normal_iterator<_Iter, _Container>& __i)
        : _M_current(__i.base()) { }

    // Forward iterator requirements
    reference
    operator*() const
    {
        return *_M_current;
    }

    pointer
    operator->() const
    {
        return _M_current;
    }

    normal_iterator_type&
    operator++()
    {
        ++_M_current;
        return *this;
    }

    normal_iterator_type
    operator++(int)
    {
        return __normal_iterator(_M_current++);
    }

    // Bidirectional iterator requirements
    normal_iterator_type&
    operator--()
    {
        --_M_current;
        return *this;
    }

    normal_iterator_type
    operator--(int)
    {
        return __normal_iterator(_M_current--);
    }

    // Random access iterator requirements
    reference
    operator[](const difference_type& __n) const
    {
        return _M_current[__n];
    }

    normal_iterator_type&
    operator+=(const difference_type& __n)
    {
        _M_current += __n;
        return *this;
    }

    normal_iterator_type
    operator+(const difference_type& __n) const
    {
        return __normal_iterator(_M_current + __n);
    }

    normal_iterator_type&
    operator-=(const difference_type& __n)
    {
        _M_current -= __n;
        return *this;
    }

    normal_iterator_type
    operator-(const difference_type& __n) const
    {
        return __normal_iterator(_M_current - __n);
    }

    difference_type
    operator-(const normal_iterator_type& __i) const
    {
        return _M_current - __i._M_current;
    }

    const _Iterator&
    base() const
    {
        return _M_current;
    }
};

// forward iterator requirements
template<typename _IteratorL, typename _IteratorR, typename _Container>
inline bool
operator==(const __normal_iterator<_IteratorL, _Container>& __lhs,
           const __normal_iterator<_IteratorR, _Container>& __rhs)
{
    return __lhs.base() == __rhs.base();
}

template<typename _IteratorL, typename _IteratorR, typename _Container>
inline bool
operator!=(const __normal_iterator<_IteratorL, _Container>& __lhs,
           const __normal_iterator<_IteratorR, _Container>& __rhs)
{
    return !(__lhs == __rhs);
}

// random access iterator requirements
template<typename _IteratorL, typename _IteratorR, typename _Container>
inline bool
operator<(const __normal_iterator<_IteratorL, _Container>& __lhs,
          const __normal_iterator<_IteratorR, _Container>& __rhs)
{
    return __lhs.base() < __rhs.base();
}

template<typename _IteratorL, typename _IteratorR, typename _Container>
inline bool
operator>(const __normal_iterator<_IteratorL, _Container>& __lhs,
          const __normal_iterator<_IteratorR, _Container>& __rhs)
{
    return __rhs < __lhs;
}

template<typename _IteratorL, typename _IteratorR, typename _Container>
inline bool
operator<=(const __normal_iterator<_IteratorL, _Container>& __lhs,
           const __normal_iterator<_IteratorR, _Container>& __rhs)
{
    return !(__rhs < __lhs);
}

template<typename _IteratorL, typename _IteratorR, typename _Container>
inline bool
operator>=(const __normal_iterator<_IteratorL, _Container>& __lhs,
           const __normal_iterator<_IteratorR, _Container>& __rhs)
{
    return !(__lhs < __rhs);
}

template<typename _Iterator, typename _Container>
inline __normal_iterator<_Iterator, _Container>
operator+(__normal_iterator<_Iterator, _Container>::difference_type __n,
          const __normal_iterator<_Iterator, _Container>& __i)
{
    return __normal_iterator<_Iterator, _Container>(__i.base() + __n);
}
```
##type_traits
sgi将之定义在type_traits.h中。
type_traits用来提取type的特性：是否具备non-trivial默认构造函数，是否具备non-trivial拷贝构造函数，是否具备non-trivial赋值操作符，是否具备non-trivial析构函数。所谓trivial，是指不发生获取和释放内存的操作或操作其他数据的操作，这样对于构造和赋值就可以使用快速的memcpy()或memmove()，对于析构就可以直接忽略。
在uninitialized_copy()等内存基本处理工具里用到了type_traits。
POD：Plain Old Data，基本的数据类型都是POD，比如int，struct，指针等。POD的默认构造，拷贝构造，赋值，析构都是trivial的。
模板类的5个类型都被定义为__false_type，假定这几个函数都是non-trivial，然后针对常见的基本数据类型做了特化，我省略了很多，因为都是类似的。
```cpp
template <bool _Truth> struct _Bool {};
typedef _Bool<true>  __true_type;
typedef _Bool<false> __false_type;

template <class _Tp>
struct __type_traits
{
    typedef __true_type     this_dummy_member_must_be_first;
    /* Do not remove this member. It informs a compiler which
       automatically specializes __type_traits that this
       __type_traits template is special. It just makes sure that
       things work if an implementation is using a template
       called __type_traits for something unrelated. */

    /* The following restrictions should be observed for the sake of
       compilers which automatically produce type specific specializations
       of this class:
           - You may reorder the members below if you wish
           - You may remove any of the members below if you wish
           - You must not rename members without making the corresponding
             name change in the compiler
           - Members you add will be treated like regular members unless
             you add the appropriate support in the compiler. */


    typedef __false_type    has_trivial_default_constructor;
    typedef __false_type    has_trivial_copy_constructor;
    typedef __false_type    has_trivial_assignment_operator;
    typedef __false_type    has_trivial_destructor;
    typedef __false_type    is_POD_type;
};


// Provide some specializations.
template<> struct __type_traits<bool>
{
    typedef __true_type    has_trivial_default_constructor;
    typedef __true_type    has_trivial_copy_constructor;
    typedef __true_type    has_trivial_assignment_operator;
    typedef __true_type    has_trivial_destructor;
    typedef __true_type    is_POD_type;
};

template<> struct __type_traits<char>
{
    typedef __true_type    has_trivial_default_constructor;
    typedef __true_type    has_trivial_copy_constructor;
    typedef __true_type    has_trivial_assignment_operator;
    typedef __true_type    has_trivial_destructor;
    typedef __true_type    is_POD_type;
};

template<> struct __type_traits<int>
{
    typedef __true_type    has_trivial_default_constructor;
    typedef __true_type    has_trivial_copy_constructor;
    typedef __true_type    has_trivial_assignment_operator;
    typedef __true_type    has_trivial_destructor;
    typedef __true_type    is_POD_type;
};

template<> struct __type_traits<float>
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
```