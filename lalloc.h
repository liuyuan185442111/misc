#ifndef LALLOC
#define LALLOC
//一个不使用默认内存分配器的string和map

#include <stddef.h>
#include <stdlib.h>

template <typename _Tp>
class lalloc
{
public:
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef _Tp*       pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp&       reference;
    typedef const _Tp& const_reference;
    typedef _Tp        value_type;

    template <class _Tp1> struct rebind
    {
        typedef lalloc<_Tp1> other;
    };

    lalloc() throw() {}
    lalloc(const lalloc&) throw() {}
    template <class _Tp1> lalloc(const lalloc<_Tp1>&) throw() {}
    ~lalloc() throw() {}

    pointer address(reference __x) const
    {
		return reinterpret_cast<_Tp*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(__x)));
    }
    const_pointer address(const_reference __x) const
    {
		return reinterpret_cast<_Tp*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(__x)));
    }

    _Tp* allocate(size_type __n, const void* = 0)
    {
		return (_Tp*)malloc(__n * sizeof(_Tp));
    }

    void deallocate(pointer __p, size_type __n)
    {
		free(__p);
    }

    size_type max_size() const throw()
    {
        return size_t(-1) / sizeof(_Tp);
    }

    void construct(pointer __p, const _Tp& __val)
    {
        new(__p) _Tp(__val);
    }
    void destroy(pointer __p)
    {
        __p->~_Tp();
    }

	//gcc basic_string need it
	bool operator==(const lalloc &rhs) const
	{
		return true;
	}
};

#include <string>
typedef std::basic_string<char, std::char_traits<char>, lalloc<char> > lstring;

#include <map>
template <typename K, typename V>
class lmap : public std::map<K, V, std::less<K>, lalloc<std::pair<const K,V> > >
{
};

#endif
