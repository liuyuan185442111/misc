basic_string在basic_string.h中定义，部分成员函数在basic_string.tcc中定义。

	template <typename _CharT, typename _Traits, typename _Alloc>
	class basic_string;
##COPY-ON-WRITE
如果直接去看源码，会比较难理解，因为basic_string的实现使用了COPY-ON-WRITE技术。关于COPY-ON-WRITE技术可移步

> [《关于COPY-ON-WRITE 》](http://blog.sina.com.cn/s/blog_6211cd0501011a9a.html)

sgi在basic_string内部定义了一个Rep的结构体，负责COPY-ON-WRITE的实现。要实现COPY-ON-WRITE，就必须对分配的内存块进行计数，Rep就用来进行计数，并把它放在每个内存块的起始位置。同时Rep里还存有一些状态信息。
所以内存布局是这样的，Rep|char_type, char_type, char_type……

此外basic_string内部还定义了一个继承自Alloc的结构体Alloc_hider，它有一个数据成员_M_p，_M_p指向实际的数据。然后定义了一个Alloc_hider类型的数据成员M_dataplus。 
```
A string looks like this：
                              	[_Rep]
                              	_M_length
[basic_string<char_type>]		_M_capacity
_M_dataplus                		_M_state
_M_p--------------------------->一个未命名的char_type数组

_M_p指向string的第一个字符, 将它强制转换为pointer-to-Rep后，再减1便得到内存块的起始地址
```
下面是上述的相关源码：
```
private:
struct _Rep
{
    typedef typename _Alloc::rebind<char>::other _Raw_bytes_alloc;

    static const size_type 	_S_max_size;
    static const _CharT 	_S_terminal;
    size_type 		_M_length;
    size_type 		_M_capacity;
    _Atomic_word	_M_references;

    bool _M_is_leaked() const
    {
        return _M_references < 0;
    }

    bool _M_is_shared() const
    {
        return _M_references > 0;
    }

    void _M_set_leaked()
    {
        _M_references = -1;
    }

    void _M_set_sharable()
    {
        _M_references = 0;
    }

    // 数据块的实际地址
    _CharT *_M_refdata() throw()
    {
        return reinterpret_cast<_CharT*> (this + 1);
    }

    _CharT &operator[](size_t __s) throw()
    {
        return _M_refdata()[__s];
    }

	// 创建可容纳capacity个CharT的内存块, 需额外考虑起始的Rep和终止的空字符
    static _Rep* _S_create(size_t __capacity, const _Alloc& __alloc)
    {
        size_t __size = (__capacity + 1) * sizeof(_CharT) + sizeof(_Rep);
        void* __place = _Raw_bytes_alloc(__alloc).allocate(__size);
        _Rep *__p = new (__place) _Rep;
        __p->_M_capacity = __capacity;
        __p->_M_set_sharable();
        __p->_M_length = 0;
        return __p;
    }

	// 释放
    void _M_dispose(const _Alloc& __a)
    {
        if (__exchange_and_add(&_M_references, -1) <= 0)
            _M_destroy(__a);
    }

	// 销毁
    void _M_destroy(const _Alloc& __a) throw();
    {
        size_type __size = sizeof(_Rep) + (_M_capacity + 1) * sizeof(_CharT);
        _Raw_bytes_alloc(__a).deallocate(reinterpret_cast<char*>(this), __size);
    }

	// 后文说明
    _CharT *_M_grab(const _Alloc& __alloc1, const _Alloc& __alloc2)
    {
        return (!_M_is_leaked() && __alloc1 == __alloc2) ?
               _M_refcopy() : _M_clone(__alloc1);
    }

	// 引用复制
    _CharT* _M_refcopy() throw()
    {
        __atomic_add(&_M_references, 1);
        return _M_refdata();
    }

	// 克隆, 需复制内存空间
    _CharT* _M_clone(const _Alloc& __alloc, size_type __res = 0);
    {
        _Rep* __r = _Rep::_S_create(_M_length + __res, __alloc);
        if (_M_length)
        {
	        // 非空字符串
            try
            {
                traits_type::copy(__r->_M_refdata(), _M_refdata(), _M_length);
            }
            catch(...)
            {
                __r->_M_destroy(__alloc);
                throw;
            }
        }
        __r->_M_length = _M_length;
        return __r->_M_refdata();
    }
};


struct _Alloc_hider : _Alloc
{
    _Alloc_hider(_CharT* __dat, const _Alloc& __a) : _Alloc(__a), _M_p(__dat) { }
    _CharT* _M_p; // The actual data
};

mutable _Alloc_hider 	_M_dataplus;
```
###成员变量
Rep有5个成员变量
_S_max_size, 字符串的最大长度, max_size()将返回这个值
_S_terminal, 字符串的结束标志, 被初始化为_CharT(), 也就是0
_M_length, 字符串的长度
_M_capacity, 字符串的容量, capacity()将返回这个值
_M_references, 引用计数

_M_capacity被初始化为(((npos - sizeof(_Rep))/sizeof(_CharT)) - 1) / 4，
由npos = sizeof(_Rep) + (m * sizeof(_CharT)) + sizeof(_CharT)得
m = ((npos - sizeof(_Rep))/sizeof(CharT)) - 1，本实现取这个值的1/4。

_M_capacity不小于_M_length，实际分配的空间为(__capacity + 1) * sizeof(_CharT) + sizeof(_Rep)

string包含_M_length + 1个字符，1就是结尾的空字符

_M_references有3种状态:
-1：一个引用，允许修改
0：一个引用，可共享，允许修改
n>0：n+1个引用，不允许修改，其他允许的操作需要加锁
###原子操作
```
_Atomic_word __attribute__((__unused__))
__exchange_and_add(volatile _Atomic_word*, int);
void __attribute__((__unused__))
__atomic_add(volatile _Atomic_word*, int);
```
它们在atomicity.h中定义，都将第2个参数加到第1个参数上，区别在于前者返回第一个参数的初始值。
`__attribute__((__unused__))`表示此函数可能用不到，用来消除编译器警告。
###_M_grab函数
在这个函数中将判断是进行引用计数加1还是重新建立一个新的字符串。必须说明的该函数只有在basic_string的copy ctor和assignment中才可能被调用，也就是说只有在新的字符串按copy或者赋值创建的时候才考虑使用引用计数。
进行refcopy或者clone的关键标识是：首先没有内存泄漏标志（关于这个标志主要是禁止string再次被共享），然后就是两个string对象的分配相同。
##构造和析构
basic_string定义了7(或9)个构造函数, 在某些情况下, gcc使用了重载, 而没有使用标准指示的默认参数。所以标准说明是7个构造函数, 这里是9个构造函数。
```
// 默认构造函数, 创建一个空字符串
inline basic_string();
explicit basic_string(const _Alloc& __a);

// 复制构造函数
basic_string(const basic_string& str);

// 由子串构造, 串+位置+长度
basic_string(const basic_string& __str, size_type __pos, size_type __n = npos);
basic_string(const basic_string& __str, size_type __pos, size_type __n, const _Alloc& __a);

// 由c-string构造
basic_string(const charT* s, const allocator_type& alloc = allocator_type());

// 由buffer构造, 指针+长度
basic_string(const charT* s, size_type n,
             const allocator_type& alloc = allocator_type());

// fill constructor, 用n个字符c来填充
basic_string(size_type n, charT c,
             const allocator_type& alloc = allocator_type());

// range constructor, 用迭代器范围[first,last)来填充
template <class InputIterator>
    basic_string(InputIterator first, InputIterator last,
                 const allocator_type& alloc = allocator_type());

// 析构函数
~basic_string()
{
    _M_rep()->_M_dispose(this->get_allocator());
}
```
构造函数在内部又调用了_S_construct()，最终是通过调用_Rep::_S_create()来完成的内存分配，然后将首字符的地址存于_M_dataplus._M_p。
##内部函数
basic_string定义了一些private的函数，供其他函数使用。
```
// 一个空串所占用的内存块
// 先加上sizeof(size_type)-1是考虑sizeof(_Rep)+sizeof(_CharT)不是sizeof(size_type)的整数倍的情况
// 为什么不直接用char数组来表示呢?
static size_type _S_empty_rep_storage[(sizeof(_Rep) + sizeof(_CharT) + sizeof(size_type) - 1)/sizeof(size_type)];

// 串的实际地址
_CharT *_M_data() const
{
    return _M_dataplus._M_p;
}

// 设置串
_CharT *_M_data(_CharT* __p)
{
    return (_M_dataplus._M_p = __p);
}

// 得到Rep的地址
_Rep *_M_rep() const
{
    return &((reinterpret_cast<_Rep*> (_M_data()))[-1]);
}

// 与begin/end类似的函数, 但没有调用_M_leak, 内部使用
iterator _M_ibegin() const
{
    return iterator(_M_data());
}

iterator _M_iend() const
{
    return iterator(_M_data() + this->size());
}

// 将串设置为leaked, 以便对其进行修改
// 在begin()和non-const op[]里使用
void _M_leak()
{
    if (!_M_rep()->_M_is_leaked())
        _M_leak_hard();
}

// 返回pos位置的迭代器, 有越界检查
iterator _M_check(size_type __pos) const
{
    if (__pos > this->size())
        __throw_out_of_range("basic_string::_M_check");
    return _M_ibegin() + __pos;
}

// pos位置再加上off的便宜量, 需确保pos位置有效
iterator _M_fold(size_type __pos, size_type __off) const
{
    size_type __newoff = __off < this->size() - __pos ? __off : this->size() - __pos;
    return (_M_ibegin() + __pos + __newoff);
}

// 泛化的拷贝
template<class _Iterator>
static void _S_copy_chars(_CharT* __p, _Iterator __k1, _Iterator __k2)
{
    for (; __k1 != __k2; ++__k1, ++__p)
        traits_type::assign(*__p, *__k1);
}

static void _S_copy_chars(_CharT* __p, iterator __k1, iterator __k2)
{
    _S_copy_chars(__p, __k1.base(), __k2.base());
}

static void _S_copy_chars(_CharT* __p, const_iterator __k1, const_iterator __k2)
{
    _S_copy_chars(__p, __k1.base(), __k2.base());
}

static void _S_copy_chars(_CharT* __p, _CharT* __k1, _CharT* __k2)
{
    traits_type::copy(__p, __k1, __k2 - __k1);
}

static void _S_copy_chars(_CharT* __p, const _CharT* __k1, const _CharT* __k2)
{
    traits_type::copy(__p, __k1, __k2 - __k1);
}

// 将字符串分为3部分: begin---pos---pos+len1---end
// 此函数将中间长度为len1的部分的长度变为len2
// 前后两部分的数据内容不变, 中间部分数据不处理
void _M_mutate(size_type __pos, size_type __len1, size_type __len2)
{
    size_type       __old_size = this->size();
    const size_type __new_size = __old_size + __len2 - __len1;
    const _CharT*        __src = _M_data()  + __pos + __len1;
    const size_type __how_much = __old_size - __pos - __len1;

    if (_M_rep()->_M_is_shared() || __new_size > capacity())
    {
        // 如果有多个引用或len2>len1, 则必须reallocate
        allocator_type __a = get_allocator();
        _Rep* __r = _Rep::_S_create(__new_size, __a);
        try
        {
            if (__pos)
                traits_type::copy(__r->_M_refdata(), _M_data(), __pos);
            if (__how_much)
                traits_type::copy(__r->_M_refdata() + __pos + __len2,
                                  __src, __how_much);
        }
        catch(...)
        {
            __r->_M_dispose(get_allocator());
            throw;
        }
        _M_rep()->_M_dispose(__a);
        _M_data(__r->_M_refdata());
    }
    else if (__how_much && __len1 != __len2)
    {
        // 原地操作
        traits_type::move(_M_data() + __pos + __len2, __src, __how_much);
    }
    _M_rep()->_M_set_sharable();
    _M_rep()->_M_length = __new_size;
    _M_data()[__new_size] = _Rep::_S_terminal;
}

// 设置为leaked, 如果有多个引用, 则做一份clone, 如果只有一个应用, 直接设置为leaked
void _M_leak_hard()
{
    if (_M_rep()->_M_is_shared())
        _M_mutate(0, 0, 0);
    _M_rep()->_M_set_leaked();
}

// 返回空串
static _Rep &_S_empty_rep()
{
    return *reinterpret_cast<_Rep*>(&_S_empty_rep_storage);
}
```
##迭代器
共8个, 在begin()和end()中, 由于可能会修改串, 所以需要先置为leaked。
```
iterator begin()
{
    _M_leak();
    return iterator(_M_data());
}

const_iterator begin() const
{
    return const_iterator(_M_data());
}

iterator end()
{
    _M_leak();
    return iterator(_M_data() + this->size());
}

const_iterator end() const
{
    return const_iterator(_M_data() + this->size());
}

reverse_iterator rbegin()
{
    return reverse_iterator(this->end());
}

const_reverse_iterator rbegin() const
{
    return const_reverse_iterator(this->end());
}

reverse_iterator rend()
{
    return reverse_iterator(this->begin());
}

const_reverse_iterator rend() const
{
    return const_reverse_iterator(this->begin());
}
```
##element access
在non-const operator[]中，由于可能会对串进行修改，所以需要先置为leaked。
operator[]和at()的功能相同，不过at()做了越界检查。
```
const_reference operator[](size_type __pos) const
{
    return _M_data()[__pos];
}

reference operator[](size_type __pos)
{
    _M_leak();
    return _M_data()[__pos];
}

const_reference at(size_type __n) const
{
    if (__n >= this->size())
        __throw_out_of_range("basic_string::at");
    return _M_data()[__n];
}

reference at(size_type __n)
{
    if (__n >= size())
        __throw_out_of_range("basic_string::at");
    _M_leak();
    return _M_data()[__n];
}
```
##capacity
与capacity相关的一些操作，比如得到长度，resize等。
```
 size_type size() const
{
    return _M_rep()->_M_length;
}

size_type length() const
{
    return _M_rep()->_M_length;
}

size_type max_size() const
{
    return _Rep::_S_max_size;
}

void resize(size_type __n, _CharT __c)
{
    size_type __size = this->size();
    if (__size < __n)
        this->append(__n - __size, __c);
    else if (__n < __size)
        this->erase(__n);
}

void resize(size_type __n)
{
    this->resize(__n, _CharT());
}

size_type capacity() const
{
    return _M_rep()->_M_capacity;
}

// 请求改变capacity, 避免减小capacity
void reserve(size_type __res = 0)
{
    if (__res > this->capacity() || _M_rep()->_M_is_shared())
    {
        allocator_type __a = get_allocator();
        _CharT* __tmp = _M_rep()->_M_clone(__a, __res - this->size());
        _M_rep()->_M_dispose(__a);
        _M_data(__tmp);
    }
}
// gcc 4.7.1里的版本
void reserve(size_type __res = 0)
{
    if (__res != this->capacity() || _M_rep()->_M_is_shared())
    {
        // Make sure we don't shrink below the current size
        if (__res < this->size())
            __res = this->size();
        const allocator_type __a = get_allocator();
        _CharT* __tmp = _M_rep()->_M_clone(__a, __res - this->size());
        _M_rep()->_M_dispose(__a);
        _M_data(__tmp);
    }
}

// 销毁串里的数据, 我觉得作用不大
void clear()
{
    _M_mutate(0, this->size(), 0);
}

// 判断串是否为空
bool empty() const
{
    return this->size() == 0;
}
```
##string operations
主要有查找操作。
```
const _CharT *c_str() const
{
    size_type __n = this->size();
    // 这句太保守, 结尾空字符一直都有
    traits_type::assign(_M_data()[__n], _Rep::_S_terminal);
    return _M_data();
}

const _CharT *data() const
{
    return _M_data();
}

allocator_type get_allocator() const
{
    return _M_dataplus;
}

// 拷贝一个子串到s, 这个子串包含从pos开始的n个字符
// 注意: 此函数不为拷贝内容末尾添加null
size_type copy(_CharT* __s, size_type __n, size_type __pos = 0) const
{
    if (__n > this->size() - __pos)
        __n = this->size() - __pos;
    traits_type::copy(__s, _M_data() + __pos, __n);
    return __n;
}

/**
*  find()查找参数给出的序列在字符串中第一次出现的位置
*  rfind()查找参数给出的序列在字符串中最后出现的位置
*/
// 从pos位置开始查找, 与s[0...n-1]相同的子串的位置
size_type find(const _CharT* __s, size_type __pos, size_type __n) const
{
    size_type __size = this->size();
    size_t __xpos = __pos;
    const _CharT* __data = _M_data();
    for (; __xpos + __n <= __size; ++__xpos)
        if (traits_type::compare(__data + __xpos, __s, __n) == 0)
            return __xpos;
    return npos;
}

// 从pos位置开始查找, 与str相同的子串的位置
size_type find(const basic_string& __str, size_type __pos = 0) const
{
    return this->find(__str.data(), __pos, __str.size());
}

// 从pos位置开始查找, 与s相同的子串的位置
size_type find(const _CharT* __s, size_type __pos = 0) const
{
    return this->find(__s, __pos, traits_type::length(__s));
}

// 从pos位置开始查找, 与c相同的字符的位置
size_type find(_CharT __c, size_type __pos = 0) const
{
    size_type __size = this->size();
    size_type __ret = npos;
    if (__pos < __size)
    {
        const _CharT* __data = _M_data();
        size_type __n = __size - __pos;
        const _CharT* __p = traits_type::find(__data + __pos, __n, __c);
        if (__p)
            __ret = __p - __data;
    }
    return __ret;
}

size_type rfind(const basic_string& __str, size_type __pos = npos) const
{
    return this->rfind(__str.data(), __pos, __str.size());
}

// 从pos位置开始向左查找, 与s[0...n-1]相同的子串的位置
size_type
rfind(const _CharT* __s, size_type __pos, size_type __n) const
{
    size_type __size = this->size();
    if (__n <= __size)
    {
        __pos = std::min(__size - __n, __pos);
        const _CharT* __data = _M_data();
        do
        {
            if (traits_type::compare(__data + __pos, __s, __n) == 0)
                return __pos;
        }
        while (__pos-- > 0);
    }
    return npos;
}

size_type rfind(const _CharT* __s, size_type __pos = npos) const
{
    return this->rfind(__s, __pos, traits_type::length(__s));
}

size_type rfind(_CharT __c, size_type __pos = npos) const
{
    size_type __size = this->size();
    if (__size)
    {
        size_t __xpos = __size - 1;
        if (__xpos > __pos)
            __xpos = __pos;

        for (++__xpos; __xpos-- > 0; )
            if (traits_type::eq(_M_data()[__xpos], __c))
                return __xpos;
    }
    return npos;
}

// 从pos位置开始, 在字符串中查找第一个字符, 这个字符在序列s[0...n-1]中, 返回这个字符的位置
size_type find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
{
    if (__n == 0) return npos;
    for (; __pos < this->size(); ++__pos)
    {
        const _CharT* __p = traits_type::find(__s, __n, _M_data()[__pos]);
        if (__p)
            return __pos;
    }
    return npos;
}

size_type find_first_of(const basic_string& __str, size_type __pos = 0) const
{
    return this->find_first_of(__str.data(), __pos, __str.size());
}

size_type find_first_of(const _CharT* __s, size_type __pos = 0) const
{
    return this->find_first_of(__s, __pos, traits_type::length(__s));
}

// 特殊情况
size_type find_first_of(_CharT __c, size_type __pos = 0) const
{
    return this->find(__c, __pos);
}

// 从pos位置向左, 在字符串中查找第一个字符, 这个字符在序列s[0...n-1]中, 返回这个字符的位置
size_type find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
{
    size_type __size = this->size();
    if (__size && __n)
    {
        if (--__size > __pos)
            __size = __pos;
        do
        {
            if (traits_type::find(__s, __n, _M_data()[__size]))
                return __size;
        }
        while (__size-- != 0);
    }
    return npos;
}

size_type find_last_of(const basic_string& __str, size_type __pos = npos) const
{
    return this->find_last_of(__str.data(), __pos, __str.size());
}

size_type find_last_of(const _CharT* __s, size_type __pos = npos) const
{
    return this->find_last_of(__s, __pos, traits_type::length(__s));
}

size_type find_last_of(_CharT __c, size_type __pos = npos) const
{
    return this->rfind(__c, __pos);
}

// find_first_not_of()和find_first_of()类似, 不过判断条件相反, 便不再赘述

// find_last_not_of()和find_last_of(), 不过判断条件相反, 便不再赘述

// 获取一个子串
basic_string substr(size_type __pos = 0, size_type __n = npos) const
{
    if (__pos > this->size())
        __throw_out_of_range("basic_string::substr");
    return basic_string(*this, __pos, __n);
}

int compare(const basic_string& __str) const
{
    size_type __size = this->size();
    size_type __osize = __str.size();
    size_type __len = min(__size, __osize);

    int __r = traits_type::compare(_M_data(), __str.data(), __len);
    if (!__r)
        __r =  __size - __osize;
    return __r;
}

// 从pos位置开始的n个字符, 和str做比较
int compare(size_type __pos, size_type __n, const basic_string& __str) const
{
    size_type __size = this->size();
    size_type __osize = __str.size();
    if (__pos > __size)
        __throw_out_of_range("basic_string::compare");

    size_type __rsize= min(__size - __pos, __n);
    size_type __len = min(__rsize, __osize);
    int __r = traits_type::compare(_M_data() + __pos, __str.data(), __len);
    if (!__r)
        __r = __rsize - __osize;
    return __r;
}

// 从pos1位置开始的n1个字符, 和str的pos2位置开始的n2个字符做比较
int compare(size_type __pos1, size_type __n1, const basic_string& __str,
            size_type __pos2, size_type __n2) const
{
    size_type __size = this->size();
    size_type __osize = __str.size();
    if (__pos1 > __size || __pos2 > __osize)
        __throw_out_of_range("basic_string::compare");

    size_type __rsize = min(__size - __pos1, __n1);
    size_type __rosize = min(__osize - __pos2, __n2);
    size_type __len = min(__rsize, __rosize);
    int __r = traits_type::compare(_M_data() + __pos1,
                                   __str.data() + __pos2, __len);
    if (!__r)
        __r = __rsize - __rosize;
    return __r;
}

int compare(const _CharT* __s) const
{
    size_type __size = this->size();
    int __r = traits_type::compare(_M_data(), __s, __size);
    if (!__r)
        __r = __size - traits_type::length(__s);
    return __r;
}

int compare(size_type __pos, size_type __n1, const _CharT* __s) const
{
    size_type __size = this->size();
    if (__pos > __size)
        __throw_out_of_range("basic_string::compare");

    size_type __osize = traits_type::length(__s);
    size_type __rsize = min(__size - __pos, __n1);
    size_type __len = min(__rsize, __osize);
    int __r = traits_type::compare(_M_data() + __pos, __s, __len);
    if (!__r)
        __r = __rsize - __osize;
    return __r;
}

int compare(size_type __pos, size_type __n1,
            const _CharT* __s, size_type __n2) const
{
    size_type __size = this->size();
    if (__pos > __size)
        __throw_out_of_range("basic_string::compare");

    size_type __osize = min(traits_type::length(__s), __n2);
    size_type __rsize = min(__size - __pos, __n1);
    size_type __len = min(__rsize, __osize);
    int __r = traits_type::compare(_M_data() + __pos, __s, __len);
    if (!__r)
        __r = __rsize - __osize;
    return __r;
}
```
##modifiers
这些函数会对字符串进行修改。
```
void push_back(_CharT __c)
{
    this->replace(_M_iend(), _M_iend(), 1, __c);
}

void swap(basic_string& __s)
{
    if (_M_rep()->_M_is_leaked())
        _M_rep()->_M_set_sharable();
    if (__s._M_rep()->_M_is_leaked())
        __s._M_rep()->_M_set_sharable();
    if (this->get_allocator() == __s.get_allocator())
    {
        _CharT* __tmp = _M_data();
        _M_data(__s._M_data());
        __s._M_data(__tmp);
    }
    else
    {
        basic_string __tmp1(_M_ibegin(), _M_iend(), __s.get_allocator());
        basic_string __tmp2(__s._M_ibegin(), __s._M_iend(), this->get_allocator());
        *this = __tmp2;
        __s = __tmp1;
    }
}

/**
* 以下是replace(), 有太多重载版本了
* 主要有两个版本, 一个接受四个参数, 前两个是字符串的迭代器, 后两个是要替换内容的迭代器;
* 第二个接受四个参数, 前两个是字符串的迭代器, 第三个是size_type类型, 第四个是_CharT类型, 表示替换内容为多个相同字符.
* 其他版本都是调用前两个版本!
*/
basic_string&
replace(iterator __i1, iterator __i2, size_type __n2, _CharT __c)
{
    size_type __n1 = __i2 - __i1;
    size_type __off1 = __i1 - _M_ibegin();
    _M_mutate (__off1, __n1, __n2);
    if (__n2)
        traits_type::assign(_M_data() + __off1, __n2, __c);
    return *this;
}

template<class _InputIterator>
basic_string&
replace(iterator __i1, iterator __i2,
        _InputIterator __k1, _InputIterator __k2)
{
    return _M_replace(__i1, __i2, __k1, __k2,
                      typename iterator_traits<_InputIterator>::iterator_category());
}

template<class _InputIterator>
basic_string&
_M_replace(iterator __i1, iterator __i2, _InputIterator __k1,
           _InputIterator __k2, input_iterator_tag)
{
    basic_string __s(__k1, __k2);
    return this->replace(__i1, __i2, __s._M_ibegin(), __s._M_iend());
}

// 将[i1,i2)替换为[k1,k2)
template<class _FwdIterator>
basic_string &
_M_replace(iterator __i1, iterator __i2,
           _FwdIterator __k1, _FwdIterator __k2, forward_iterator_tag)
{
    size_type __dold = __i2 - __i1;
    size_type __dnew = static_cast<size_type>(distance(__k1, __k2));

    size_type __off = __i1 - _M_ibegin();
    if (__dnew)
        _S_copy_chars(_M_data() + __off, __k1, __k2);

    return *this;
}

basic_string&
replace(iterator __i1, iterator __i2, const basic_string& __str)
{
    return this->replace(__i1, __i2, __str.begin(), __str.end());
}

basic_string&
replace(iterator __i1, iterator __i2,
        const _CharT* __s, size_type __n)
{
    return this->replace(__i1, __i2, __s, __s + __n);
}

basic_string&
replace(iterator __i1, iterator __i2, const _CharT* __s)
{
    return this->replace(__i1, __i2, __s,
                         __s + traits_type::length(__s));
}

// replace [pos,pos+n) with str
basic_string&
replace(size_type __pos, size_type __n, const basic_string& __str)
{
    return this->replace(_M_check(__pos), _M_fold(__pos, __n),
                         __str.begin(), __str.end());
}

// replace [pos,pos+n1) with str [s,s+n2)
basic_string&
replace(size_type __pos, size_type __n1, const _CharT* __s,
        size_type __n2)
{
    return this->replace(_M_check(__pos), _M_fold(__pos, __n1),
                         __s, __s + __n2);
}

// replace [pos,pos+n1) with s
basic_string&
replace(size_type __pos, size_type __n1, const _CharT* __s)
{
    return this->replace(_M_check(__pos), _M_fold(__pos, __n1),
                         __s, __s + traits_type::length(__s));
}

// replace [pos,pos+n1) with n2 x c
basic_string&
replace(size_type __pos, size_type __n1, size_type __n2, _CharT __c)
{
    return this->replace(_M_check(__pos), _M_fold(__pos, __n1), __n2, __c);
}

// replace [pos1,pos1+n1) with [pos2,pos2+n2) of str
basic_string&
replace(size_type __pos1, size_type __n1, const basic_string& __str,
        size_type __pos2, size_type __n2)
{
    return this->replace(_M_check(__pos1), _M_fold(__pos1, __n1),
                         __str._M_check(__pos2), __str._M_fold(__pos2, __n2));
}

/**
* 以下是append()
* 实质是调用replace(), 前两个参数固定为_M_iend()
*/
basic_string&
append(const basic_string& __str, size_type __pos, size_type __n)
{
    // If appending itself, string needs to pre-reserve the
    // correct size so that _M_mutate does not clobber the
    // iterators formed here.
    size_type __len = min(__str.size() - __pos, __n) + this->size();
    if (__len > this->capacity())
        this->reserve(__len);
    return this->replace(_M_iend(), _M_iend(), __str._M_check(__pos),
                         __str._M_fold(__pos, __n));
}

basic_string&
append(const _CharT* __s, size_type __n)
{
    size_type __len = __n + this->size();
    if (__len > this->capacity())
        this->reserve(__len);
    return this->replace(_M_iend(), _M_iend(), __s, __s + __n);
}

basic_string&
append(size_type __n, _CharT __c)
{
    size_type __len = __n + this->size();
    if (__len > this->capacity())
        this->reserve(__len);
    return this->replace(_M_iend(), _M_iend(), __n, __c);
}

basic_string&
append(const _CharT* __s)
{
    return this->append(__s, traits_type::length(__s));
}

template<class _InputIterator>
basic_string&
append(_InputIterator __first, _InputIterator __last)
{
    return this->replace(_M_iend(), _M_iend(), __first, __last);
}

/**
* 以下是assign(), 很多也调用了replace()
*/
// *this <= str
basic_string&
assign(const basic_string& __str)
{
    if (_M_rep() != __str._M_rep())
    {
        allocator_type __a = this->get_allocator();
        _CharT* __tmp = __str._M_rep()->_M_grab(__a, __str.get_allocator());
        _M_rep()->_M_dispose(__a);
        _M_data(__tmp);
    }
    return *this;
}

// *this <= [first,last)
template<class _InputIterator>
basic_string&
assign(_InputIterator __first, _InputIterator __last)
{
    return this->replace(_M_ibegin(), _M_iend(), __first, __last);
}

basic_string&
assign(const basic_string& __str, size_type __pos, size_type __n)
{
    return this->assign(__str._M_check(__pos), __str._M_fold(__pos, __n));
}

basic_string&
assign(const _CharT* __s, size_type __n)
{
    return this->assign(__s, __s + __n);
}

basic_string&
assign(const _CharT* __s)
{
    return this->assign(__s, __s + traits_type::length(__s));
}

basic_string&
assign(size_type __n, _CharT __c)
{
    return this->replace(_M_ibegin(), _M_iend(), __n, __c);
}

/**
* 以下是+=和=, 前者调用了append(), 后者调用了assign()
*/
basic_string&
operator+=(const basic_string& __str)
{
    return this->append(__str);
}

basic_string&
operator+=(const _CharT* __s)
{
    return this->append(__s);
}

basic_string&
operator+=(_CharT __c)
{
    return this->append(size_type(1), __c);
}

basic_string&
operator=(const basic_string& __str)
{
    return this->assign(__str);
}

basic_string&
operator=(const _CharT* __s)
{
    return this->assign(__s);
}

basic_string&
operator=(_CharT __c)
{
    return this->assign(1, __c);
}

/**
* insert()和erase()全程调用了replace(), 我都懒得贴出来了
*/
```
##非成员函数
主要是几个运算符重载。
```
template<typename _CharT, typename _Traits, typename _Alloc>
inline void
swap(basic_string<_CharT, _Traits, _Alloc>& __lhs,
     basic_string<_CharT, _Traits, _Alloc>& __rhs)
{
    __lhs.swap(__rhs);
}

// +运算符主要步骤是, 新建一个串, 调用reserve()调整容量, 再调用append(), 这里不再赘述

// 比较运算符的重载全是调用compare(), 这里不再赘述

// 下面的代码从gcc 4.7.1里拿到
// >>运算符和getline()的重载主要还是和basic_istream有关, 等到了输入输出相关内容时再看吧; <<运算符应该简单多了, 就不用看了.
template<typename _CharT, typename _Traits, typename _Alloc>
basic_istream<_CharT, _Traits>&
operator>>(basic_istream<_CharT, _Traits>& __in,
           basic_string<_CharT, _Traits, _Alloc>& __str)
{
    typedef basic_istream<_CharT, _Traits>		__istream_type;
    typedef basic_string<_CharT, _Traits, _Alloc>	__string_type;
    typedef typename __istream_type::ios_base         __ios_base;
    typedef typename __istream_type::int_type		__int_type;
    typedef typename __string_type::size_type		__size_type;
    typedef ctype<_CharT>				__ctype_type;
    typedef typename __ctype_type::ctype_base         __ctype_base;

    __size_type __extracted = 0;
    typename __ios_base::iostate __err = __ios_base::goodbit;
    typename __istream_type::sentry __cerb(__in, false);
    if (__cerb)
    {
        __try
        {
            // Avoid reallocation for common case.
            __str.erase();
            _CharT __buf[128];
            __size_type __len = 0;
            const streamsize __w = __in.width();
            const __size_type __n = __w > 0 ? static_cast<__size_type>(__w)
                                    : __str.max_size();
            const __ctype_type& __ct = use_facet<__ctype_type>(__in.getloc());
            const __int_type __eof = _Traits::eof();
            __int_type __c = __in.rdbuf()->sgetc();

            while (__extracted < __n
                    && !_Traits::eq_int_type(__c, __eof)
                    && !__ct.is(__ctype_base::space,
                                _Traits::to_char_type(__c)))
            {
                if (__len == sizeof(__buf) / sizeof(_CharT))
                {
                    __str.append(__buf, sizeof(__buf) / sizeof(_CharT));
                    __len = 0;
                }
                __buf[__len++] = _Traits::to_char_type(__c);
                ++__extracted;
                __c = __in.rdbuf()->snextc();
            }
            __str.append(__buf, __len);

            if (_Traits::eq_int_type(__c, __eof))
                __err |= __ios_base::eofbit;
            __in.width(0);
        }
        __catch(__cxxabiv1::__forced_unwind&)
        {
            __in._M_setstate(__ios_base::badbit);
            __throw_exception_again;
        }
        __catch(...)
        {
            // _GLIBCXX_RESOLVE_LIB_DEFECTS
            // 91. Description of operator>> and getline() for string<>
            // might cause endless loop
            __in._M_setstate(__ios_base::badbit);
        }
    }
    // 211.  operator>>(istream&, string&) doesn't set failbit
    if (!__extracted)
        __err |= __ios_base::failbit;
    if (__err)
        __in.setstate(__err);
    return __in;
}

template<typename _CharT, typename _Traits, typename _Alloc>
basic_istream<_CharT, _Traits>&
getline(basic_istream<_CharT, _Traits>& __in,
        basic_string<_CharT, _Traits, _Alloc>& __str, _CharT __delim)
{
    typedef basic_istream<_CharT, _Traits>		__istream_type;
    typedef basic_string<_CharT, _Traits, _Alloc>	__string_type;
    typedef typename __istream_type::ios_base         __ios_base;
    typedef typename __istream_type::int_type		__int_type;
    typedef typename __string_type::size_type		__size_type;

    __size_type __extracted = 0;
    const __size_type __n = __str.max_size();
    typename __ios_base::iostate __err = __ios_base::goodbit;
    typename __istream_type::sentry __cerb(__in, true);
    if (__cerb)
    {
        __try
        {
            __str.erase();
            const __int_type __idelim = _Traits::to_int_type(__delim);
            const __int_type __eof = _Traits::eof();
            __int_type __c = __in.rdbuf()->sgetc();

            while (__extracted < __n
                    && !_Traits::eq_int_type(__c, __eof)
                    && !_Traits::eq_int_type(__c, __idelim))
            {
                __str += _Traits::to_char_type(__c);
                ++__extracted;
                __c = __in.rdbuf()->snextc();
            }

            if (_Traits::eq_int_type(__c, __eof))
                __err |= __ios_base::eofbit;
            else if (_Traits::eq_int_type(__c, __idelim))
            {
                ++__extracted;
                __in.rdbuf()->sbumpc();
            }
            else
                __err |= __ios_base::failbit;
        }
        __catch(__cxxabiv1::__forced_unwind&)
        {
            __in._M_setstate(__ios_base::badbit);
            __throw_exception_again;
        }
        __catch(...)
        {
            // _GLIBCXX_RESOLVE_LIB_DEFECTS
            // 91. Description of operator>> and getline() for string<>
            // might cause endless loop
            __in._M_setstate(__ios_base::badbit);
        }
    }
    if (!__extracted)
        __err |= __ios_base::failbit;
    if (__err)
        __in.setstate(__err);
    return __in;
}
```
##参考
STL中string的源码解读
http://blog.csdn.net/pizi0475/article/details/5288432