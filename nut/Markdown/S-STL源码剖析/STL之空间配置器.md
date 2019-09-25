空间配置器是memory的一部分，以下提供的代码均取自或稍修改自stl_alloc.h。
##C++标准对空间配置器的规范
标准规范只定义了一个空间配置器，所有的标准容器如果他们的最后一个模板参数未被设置，将会使用这个默认空间配置器：

	template <typename T> class allocator;
任何与默认空间配置器有相同成员并且满足其最小要求的其他空间配置器也可被标准容器用作空间配置器。
根据C++标准的规范，以下是allocator的必要接口：
###成员类型
|member|definition |represents|
|-|-|-|
|value_type|T|Element type|
|pointer|T*|Pointer to element|
|reference|T&|Reference to element|
|const_pointer|const T*|Pointer to constant element|
|const_reference|const T&|Reference to constant element 
|size_type|size_t|Quantities of elements|
|difference_type|ptrdiff_t|Difference between two pointers|
|rebind&lt;Type>|一个模板类|拥有唯一成员other，那是一个typedef，代表allocator&lt;Type>|
###成员函数
```
allocator::allocator()
	默认构造函数
allocator::allocator(const allocator&)
	复制构造函数
template <typename U>
allocator::allocator(const allocator<U>&)
	泛化的复制构造函数
allocator::~allocator()
	析构函数
pointer allocator::address(reference x) const
	返回某个对象的地址。a.address(x)等同于&x
const_pointer allocator::address(const_reference x) const
	返回某个const对象的地址。a.address(x)等同于&x
pointer allocator::allocate(size_type n, const void* = 0)
	配置空间，足以存储n个T对象。第二参数是个提示，实现上可能会利用它来增进区域性，或完全忽略之
void allocator::deallocate(pointer p, size_type n)
	归还先前配置的空间
size_type allocator::max_size() const
	返回可成功配置的最大量
void allocator::construct(pointer p, const T &x)
	等同于new((void*)p) T(x)
void allocator::destroy(pointer p)
	等同于p->~T()
```
###specialization
memory头文件提供了一个void类型空间配置器的specialization，定义如下：
```
template <> class allocator<void>
{
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	typedef void value_type;
	template <class U> struct rebind { typedef allocator<U> other; };
};
```
##gcc的实现
gcc中定义的allocator完全符合标准的要求：
```
template <typename _Tp>
class allocator
{
    typedef alloc _Alloc; // 启用的配置器
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
        typedef allocator<_Tp1> other;
    };

    allocator() throw() {}
    allocator(const allocator&) throw() {}
    template <class _Tp1> allocator(const allocator<_Tp1>&) throw() {}
    ~allocator() throw() {}

    pointer address(reference __x) const
    {
        return &__x;
    }
    const_pointer address(const_reference __x) const
    {
        return &__x;
    }

    // The C++ standard 没有说明 n 为 0 时返回何值
    _Tp* allocate(size_type __n, const void* = 0)
    {
        return __n != 0 ? static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp))) : 0;
    }

    // p 不允许为空指针
    void deallocate(pointer __p, size_type __n)
    {
        _Alloc::deallocate(__p, __n * sizeof(_Tp));
    }

    // 这个实现的也太糊弄了
    size_type max_size() const throw()
    {
        return size_t(-1) / sizeof(_Tp);
    }

    // 通过 placement new 来进行构造
    void construct(pointer __p, const _Tp& __val)
    {
        new(__p) _Tp(__val);
    }
    void destroy(pointer __p)
    {
        __p->~_Tp();
    }
};

template<>
class allocator<void>
{
public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
    typedef void        value_type;

    template <class _Tp1> struct rebind
    {
        typedef allocator<_Tp1> other;
    };
};
```
allocator是提供给标准容器或其他组件的接口，实际操作是由alloc这个空间配置器来实现的。
gcc另外实现了两个空间配置器，分别是一级配置器（__malloc_alloc_template）和二级配置器（__default_alloc_template），他们需要至少提供allocate( )和deallocate( )作为接口。默认启用的是二级配置器。
```
template <int __inst>
class __malloc_alloc_template;
template <bool threads, int inst>
class __default_alloc_template;
```
通过typedef将__default_alloc_template定义为alloc：
```
typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;
typedef __default_alloc_template<false, 0> single_client_alloc;
```
用户应仅使用alloc，__default_alloc_template 在将来可能会有所修改。
为简单起见，这里忽略多线程的情况，但alloc是支持多线程的。
###一级空间配置器
一级配置器只是简单封装了malloc( )和free( )：
```
/**************************************************************
   一级配置器
**************************************************************/
template <int __inst>
class __malloc_alloc_template
{
private:
    // out-of-memory handling
    static void *_S_oom_malloc(size_t);
    static void *_S_oom_realloc(void*, size_t);
    static void (*__malloc_alloc_oom_handler)();

public:
    static void *allocate(size_t __n)
    {
        void *__result = malloc(__n);
        if (0 == __result) __result = _S_oom_malloc(__n);
        return __result;
    }

    static void deallocate(void* __p, size_t /* __n */)
    {
        free(__p);
    }

    static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
    {
        void *__result = realloc(__p, __new_sz);
        if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
        return __result;
    }

    static void (* __set_malloc_handler(void (*__f)()))()
    {
        void (*__old)() = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = __f;
        return(__old);
    }
};

template <int __inst>
void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;

template <int __inst>
void *__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
    void (*__my_malloc_handler)();
    void *__result;

    for (;;)
    {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler)
        {
            std::__throw_bad_alloc();
        }
        (*__my_malloc_handler)();
        __result = malloc(__n);
        if (__result) return(__result);
    }
}

template <int __inst>
void *__malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
{
    void (*__my_malloc_handler)();
    void *__result;

    for (;;)
    {
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler)
        {
            std::__throw_bad_alloc();
        }
        (*__my_malloc_handler)();
        __result = realloc(__p, __n);
        if (__result) return(__result);
    }
}

typedef __malloc_alloc_template<0> malloc_alloc;
```
因为malloc( )没有类似new handler机制，所以一级配置器自己实现了一个类似的机制：如果allocate( )调用malloc( )不成功，改调用oom_malloc( )，oom_malloc( )在内部会调用__my_malloc_handler( )，如果__my_malloc_handler( )未被设定，就抛出一个bad_alloc异常。
###二级空间配置器
gcc对二级空间配置器的描述是：
Important implementation properties:
1. If the client request an object of size > _MAX_BYTES, the resulting object will be obtained directly from malloc.
2. In all other cases, we allocate an object of size exactly _S_round_up(requested_size).  Thus the client has enough size information that we can return the object to the proper free list without permanently losing part of the object.

侯捷在《STL源码剖析》里对此的说明是：
提供了一个二级空间配置器，如果区块够大，超过128 bytes，就移交给第一级配置器处理。当区块小于128 bytes时，则以内存池管理，此法又称为次层配置（sub-allocation）：每次配置一大块内存，并维护对应之自由链表。下次若再有相同大小的内存需求，直接从free lists中取出。如果用户释放小额块，就由配置器回收到free-lists中。

为了方便管理，第二级配置器会主动将任何小额区块的内存量上调至8的倍数，并维护16个free lists，各自管理的大小分别为8,16,24,32,40,48,56,64,72,80,88,96,104,112,150,128 bytes的小额区块。

每个free list的节点类型是：
```
union obj
{
	union obj* free_list_link; // 用于维护空闲内存，指向下一个空闲节点  
	char client_data[1];    // 用于用户使用  
}
```
一直不能理解，直到我看到：

> 注意节点是union类型的，当节点空闲(未被分配时)，节点使用第一字段指向下一个空闲节点，当节点被分配后，用户可以直接使用第二字段，这样自由链表就不会因为free_list_link指针而造成内存的浪费（当节点被分配出去后，free_list_link指针就没有意义了）。

才恍然大悟，obj 代表这样一段内存空间，前四个字节是一个指针，当这段内存在free list中时，这个指针指向下一段内存。
但为什么是char数组？
其实，这个char数组不是必须的，我们需要的只是一个地址而已，大不了做个强制转换而已（测试代码）：

	char heap[10];
	obj *result = (obj*)heap;
	char *r = (char*)result;
但如果引入一个char数组，则不需要强制转换了，`char *r = result->client_data;`。这里的char数组也可以换成其他类型，甚至是指针类型，但此时比较不好理解，比如`char *client_data;`，则必须写成这样`char *r = (char*)(&result->client_data);`。

下面是二级配置器的实现代码（我稍微做了些调整）：
```
/**************************************************************
   二级配置器
**************************************************************/

template <bool threads, int inst>
class __default_alloc_template
{
private:
    static const int _ALIGN = 8; // 8字节对齐
    static const int _MAX_BYTES = 128; // 可管理的最大字节数
    static const int _NFREELISTS = 16; // free lists 数目 _MAX_BYTES/_ALIGN

    // 将要分配字节数提升至8的整数倍
    static size_t _S_round_up(size_t __bytes)
    {
        return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1));
    }

    union _Obj
    {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];    /* The client sees this. */
    };

    static _Obj* volatile _S_free_list[];

    // 确定应在哪个 list
    static size_t _S_freelist_index(size_t __bytes)
    {
        return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
    }

    // 重点实现代码, 后面介绍
    static void* _S_refill(size_t __n);
    static char* _S_chunk_alloc(size_t __size, int &__nobjs);

    // Chunk allocation state, chunk_alloc 里使用
    static char* _S_start_free; // 内存池起始地址
    static char* _S_end_free; // 内存池结束地址
    static size_t _S_heap_size; // 内存池大小

public:
    // 从 free list 头部取出一个大小为 n 的区块, n 大于 0
    static void *allocate(size_t __n)
    {
        void *__ret = 0;

        if (__n > (size_t) _MAX_BYTES)
        {
            // 超过 128 bytes, 直接请求一级配置器
            __ret = malloc_alloc::allocate(__n);
        }
        else
        {
            _Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__n);

            _Obj* __result = *__my_free_list;
            if (__result == 0) // list 为空
                __ret = _S_refill(_S_round_up(__n)); // 从内存池取一些
            else
            {
                *__my_free_list = __result -> _M_free_list_link;
                __ret = __result;
            }
        }

        return __ret;
    };

    // 回收区块到 free list 头部, p 不为空
    static void deallocate(void* __p, size_t __n)
    {
        if (__n > (size_t) _MAX_BYTES)
            malloc_alloc::deallocate(__p, __n);
        else
        {
            _Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__n);
            _Obj* __q = (_Obj*)__p;

            __q -> _M_free_list_link = *__my_free_list;
            *__my_free_list = __q;
        }
    }

    static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz)
    {
        void *__result;
        size_t __copy_sz;

        if (__old_sz > (size_t) _MAX_BYTES && __new_sz > (size_t) _MAX_BYTES)
        {
            return(realloc(__p, __new_sz));
        }
        if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
        __result = allocate(__new_sz);
        __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
        memcpy(__result, __p, __copy_sz);
        deallocate(__p, __old_sz);
        return(__result);
    }
};


template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj* volatile
__default_alloc_template<__threads, __inst> ::_S_free_list[
    __default_alloc_template<__threads, __inst>::_NFREELISTS
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


// 被 refill() 调用, 从内存池中取空间给 free list 使用
// 分配 nobjs 个大小为 size 的区块, 它们占用连续的内存, 实际分配的数量存储于 nobjs 中, 函数返回所分配内存的起始地址
template <bool __threads, int __inst>
char *__default_alloc_template<__threads, __inst>::
_S_chunk_alloc(size_t __size, int &__nobjs)
{
    char *__result;
    size_t __total_bytes = __size * __nobjs;
    size_t __bytes_left = _S_end_free - _S_start_free; // 内存池剩余空间

    if (__bytes_left >= __total_bytes)
    {
        // 内存池剩余空间完全满足需求量
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    }
    else if (__bytes_left >= __size)
    {
        // 内存池剩余空间不能完全满足需求量, 但能够供应至少一个区块
        __nobjs = (int)(__bytes_left/__size);
        __total_bytes = __size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    }
    else
    {
        // 内存池剩余空间连一个区块的大小能不能提供
        // 利用 malloc() 从 heap 中配置内存, 大小为需求量的两倍, 再加上一个随着配置次数增加而越来越大的附加量
        size_t __bytes_to_get = 2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
        // Try to make use of the left-over piece
        // 尝试将内存池中剩余的残余空间分配到适当的 free list 中
        // bytes_left 一定是8的整数倍
        if (__bytes_left > 0)
        {
            _Obj* volatile* __my_free_list = _S_free_list + _S_freelist_index(__bytes_left);

            ((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
            *__my_free_list = (_Obj*)_S_start_free;
        }

        // 尝试从 heap 中配置内存
        _S_start_free = (char*)malloc(__bytes_to_get);
        if (0 == _S_start_free)
        {
            // heap 空间不足, malloc() 失败, 无法获得内存
            size_t __i;
            _Obj* volatile* __my_free_list;
            _Obj* __p;
            // Try to make do with what we have.  That can't hurt.
            // We do not try smaller requests, since that tends
            // to result in disaster on multi-process machines.
            // 尝试搜寻"大小 >=size 的可用区块"
            for (__i = __size; __i <= (size_t)_MAX_BYTES; __i += (size_t)_ALIGN)
            {
                __my_free_list = _S_free_list + _S_freelist_index(__i);
                __p = *__my_free_list;
                if (0 != __p)
                {
                    *__my_free_list = __p -> _M_free_list_link;
                    _S_start_free = (char*)__p;
                    _S_end_free = _S_start_free + __i;
                    // 现在至少能提供一个区块了, 递归调用自己以修正 nobjs
                    return(_S_chunk_alloc(__size, __nobjs));
                }
            }
            
            // 连 free list 里也没有可用内存了
            _S_end_free = 0;
            // 调用一级配置器看能不能有点用
            // 一级配置器有 out-of-memory 处理机制, 或许有机会改善现在的情况, 如果无法改善, 抛出bad_alloc异常
            _S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);
        }
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
        return(_S_chunk_alloc(__size, __nobjs));
    }
}

/**
返回一个大小为 n 的对象(假定 n 已经适当上调至 8 的倍数), 而且尝试为对应的 free list 增加节点数目
在 allocate() 中, 当 free list 中没有可用区块时, 就会调用 refill() 来给 free list 添加节点
新的空间将取自内存池(经由 chunk_alloc() 完成), 缺省取得20个新节点, 但如果内存池空间不足,
获得的节点数可能小于20, 其中一个节点返回给调用者, 剩下的节点添入对应 free list
*/
template <bool __threads, int __inst>
void *__default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
    int __nobjs = 20;
    // 注意参数 nobjs 是引用类型
    char* __chunk = _S_chunk_alloc(__n, __nobjs);
    _Obj* volatile* __my_free_list;
    _Obj* __result;
    _Obj* __current_obj;
    _Obj* __next_obj;

    // 仅获得一个区块, 分配给调用者用, free list 无新节点
    if (1 == __nobjs) return(__chunk);
    
    // 将多余区块纳入 free list
    __my_free_list = _S_free_list + _S_freelist_index(__n);
    __result = (_Obj*)__chunk;
    *__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
    // 将各节点串联起来, 第0个区块将返回给调用者
    for (int __i = 1; __i < __nobjs - 1; __i++)
    {
        __current_obj = __next_obj;
        __next_obj = (_Obj*)((char*)__next_obj + __n);
        __current_obj -> _M_free_list_link = __next_obj;
    }
    __next_obj -> _M_free_list_link = 0;
    return(__result);
}
```