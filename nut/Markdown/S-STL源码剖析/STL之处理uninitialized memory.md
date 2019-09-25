memory里除了空间配置器，auto_ptr，还有一些作用于uninitialized memory的工具。
##构造和析构工具
定义于stl_construct.h文件。
```cpp
template <class _T1, class _T2>
inline void _Construct(_T1* __p, const _T2& __value)
{
    new ((void*) __p) _T1(__value);
}

template <class _T1>
inline void _Construct(_T1* __p)
{
    new ((void*) __p) _T1();
}

template <class _Tp>
inline void _Destroy(_Tp* __pointer)
{
    __pointer->~_Tp();
}

template <class _ForwardIterator>
void __destroy_aux(_ForwardIterator __first, _ForwardIterator __last, __false_type)
{
    for ( ; __first != __last; ++__first)
        _Destroy(&*__first);
}

template <class _ForwardIterator>
inline void __destroy_aux(_ForwardIterator, _ForwardIterator, __true_type) {}

template <class _ForwardIterator, class _Tp>
inline void __destroy(_ForwardIterator __first, _ForwardIterator __last, _Tp*)
{
    typedef typename __type_traits<_Tp>::has_trivial_destructor
    _Trivial_destructor;
    __destroy_aux(__first, __last, _Trivial_destructor());
}

template <class _ForwardIterator>
inline void _Destroy(_ForwardIterator __first, _ForwardIterator __last)
{
    __destroy(__first, __last, __value_type(__first));
}

inline void _Destroy(char*, char*) {}
inline void _Destroy(int*, int*) {}
inline void _Destroy(long*, long*) {}
inline void _Destroy(float*, float*) {}
inline void _Destroy(double*, double*) {}
inline void _Destroy(wchar_t*, wchar_t*) {}
```
上述construct接受一个指针和一个初值，该函数的用途就是将初值设定到指针所指的空间上。c++的placement new运算符可用来完成这一任务。
destroy有两个版本，第一版本接受一个指针，准备将该指针所指之物析构掉，直接调用该对象的析构函数即可。第二版本接受first和last两个迭代器，准备将 [first, last) 范围内的所有对象析构掉。
在第二个版本destroy()中，首先利用__value_type()（ 参考[STL之迭代器](http://blog.csdn.net/liuyuan185442111/article/details/45848299#t0)）获得迭代器所指对象的型别，再利用__type_traits（参考[STL之__normal_iterator和__type_traits](http://blog.csdn.net/liuyuan185442111/article/details/45848827)）判断该型别的析构函数是否无关痛痒。若是(__true_type) ，则什么也不做就结束；若否（__false_type），这才以循环方式巡访整个范围，并在循环中每经历一个对象就调用第一个版本的destroy()。
##内存处理工具
stl_uninitialized.h这里定义了一些全局函数，用来填充（fill）或复制（copy）大块内存数据，它们隶属于STL标准规范。
uninitialized_copy();
uninitialized_fill();
uninitialized_fill_n();
这些函数不属于空间配置器范畴，但与对象初值设置有关，对于容器的大规模元素初值设置很有帮助。这些函数对于效率都面面俱到的考虑，最差情况下回调用construct()，最佳情况则会使用c标准函数memmove()直接进行内存数据的移动。
这三个函数使我们能够将内存的配置与对象的构造行为分离开来。
这三个函数分别对应于高层次的copy()，fill()，fill_n()。
这三个函数必须具备 “commit or rollback” 语意，要么构造出所有必要元素，要么不构造任何东西。但我为了简便，下面的代码中把异常处理给去掉了。
```cpp
template <class _InputIter, class _ForwardIter>
inline _ForwardIter
__uninitialized_copy_aux(_InputIter __first, _InputIter __last,
                         _ForwardIter __result, __true_type)
{
    return copy(__first, __last, __result);
}

template <class _InputIter, class _ForwardIter>
_ForwardIter __uninitialized_copy_aux(_InputIter __first, _InputIter __last,
                                      _ForwardIter __result, __false_type)
{
    _ForwardIter __cur = __result;
    for ( ; __first != __last; ++__first, ++__cur)
        _Construct(&*__cur, *__first);
    return __cur;
}

template <class _InputIter, class _ForwardIter, class _Tp>
inline _ForwardIter
__uninitialized_copy(_InputIter __first, _InputIter __last,
                     _ForwardIter __result, _Tp*)
{
    typedef typename __type_traits<_Tp>::is_POD_type _Is_POD;
    return __uninitialized_copy_aux(__first, __last, __result, _Is_POD());
}

template <class _InputIter, class _ForwardIter>
inline _ForwardIter
uninitialized_copy(_InputIter __first, _InputIter __last,
                   _ForwardIter __result)
{
    return __uninitialized_copy(__first, __last, __result, __value_type(__result));
}

inline char*
uninitialized_copy(const char* __first, const char* __last,
                   char* __result)
{
    memmove(__result, __first, __last - __first);
    return __result + (__last - __first);
}

inline wchar_t*
uninitialized_copy(const wchar_t* __first, const wchar_t* __last,
                   wchar_t* __result)
{
    memmove(__result, __first, sizeof(wchar_t) * (__last - __first));
    return __result + (__last - __first);
}


template <class _ForwardIter, class _Tp>
inline void
__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last,
                         const _Tp& __x, __true_type)
{
    fill(__first, __last, __x);
}

template <class _ForwardIter, class _Tp>
void
__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last,
                         const _Tp& __x, __false_type)
{
    _ForwardIter __cur = __first;
    for ( ; __cur != __last; ++__cur)
        _Construct(&*__cur, __x);
}

template <class _ForwardIter, class _Tp, class _Tp1>
inline void
__uninitialized_fill(_ForwardIter __first, _ForwardIter __last,
                     const _Tp& __x, _Tp1*)
{
    typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
    __uninitialized_fill_aux(__first, __last, __x, _Is_POD());
}

template <class _ForwardIter, class _Tp>
inline void
uninitialized_fill(_ForwardIter __first, _ForwardIter __last,
                   const _Tp& __x)
{
    __uninitialized_fill(__first, __last, __x, __value_type(__first));
}


template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter
__uninitialized_fill_n_aux(_ForwardIter __first, _Size __n,
                           const _Tp& __x, __true_type)
{
    return fill_n(__first, __n, __x);
}

template <class _ForwardIter, class _Size, class _Tp>
_ForwardIter
__uninitialized_fill_n_aux(_ForwardIter __first, _Size __n,
                           const _Tp& __x, __false_type)
{
    _ForwardIter __cur = __first;
    for ( ; __n > 0; --__n, ++__cur)
        _Construct(&*__cur, __x);
    return __cur;
}

template <class _ForwardIter, class _Size, class _Tp, class _Tp1>
inline _ForwardIter
__uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x, _Tp1*)
{
    typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
    return __uninitialized_fill_n_aux(__first, __n, __x, _Is_POD());
}

template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter
uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x)
{
    return __uninitialized_fill_n(__first, __n, __x, __value_type(__first));
}
```
##raw_storage_iterator
实现在stl_raw_storage_iter.h文件中。
用来在未初始化的内存中走访并进行初始化工作，这个类一般作为迭代器的基类。
```cpp
template <class _ForwardIterator, class _Tp>
class raw_storage_iterator
{
protected:
    _ForwardIterator _M_iter;
public:
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit raw_storage_iterator(_ForwardIterator __x) : _M_iter(__x) {}
    raw_storage_iterator& operator*() { return *this; }
    raw_storage_iterator& operator=(const _Tp& __element)
    {
        new ((void*) (&*_M_iter)) (*_ForwardIterator)(__element);
        return *this;
    }
    raw_storage_iterator<_ForwardIterator, _Tp>& operator++()
    {
        ++_M_iter;
        return *this;
    }
    raw_storage_iterator<_ForwardIterator, _Tp> operator++(int)
    {
        raw_storage_iterator<_ForwardIterator, _Tp> __tmp = *this;
        ++_M_iter;
        return __tmp;
    }
};
```
##临时缓存
在stl_tempbuf.h文件中实现。
```cpp
// get_temporary_buffer为n个T分配临时内存, 并返回指向内存块的指针和实际分配的数量
template <class T>
inline pair <T*, ptrdiff_t>
get_temporary_buffer(ptrdiff_t n)
{
    if (n > ptrdiff_t(INT_MAX / sizeof(T)))
        n = INT_MAX / sizeof(T);

    while (n > 0)
    {
        T* tmp = (T*) malloc((size_t)n * sizeof(T));
        if (tmp != 0)
            return pair<T*, ptrdiff_t>(tmp, n);
        n /= 2;
    }

    return pair<T*, ptrdiff_t>((T*)0, 0);
}

// return_temporary_buffer释放内存
template <class T>
void return_temporary_buffer(T* p)
{
    free(p);
}
```