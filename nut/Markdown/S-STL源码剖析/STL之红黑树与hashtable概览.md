##红黑树，set，map
```cpp
// 红黑树, 这是一个内部容器, 不提供给外部使用,
// set/multiset和map/multimap都基于Rb_tree, 基本是转调了红黑树的接口而已.
template <typename Key, typename Value, typename KeyOfValue, typename Compare,
         typename Alloc = std::allocator<Value> >
class _Rb_tree;

// set
template <typename Key, typename Compare = std::less<Key>,
         typename Alloc = std::allocator<Key> >
class set;

// set内部用来存储数据的是
_Rb_tree<key_type, value_type, _Identity<key_type>, key_compare, allocator_type> _Ref;

// map
template <typename Key, typename Tp, typename Compare = std::less<Key>,
         typename Alloc = std::allocator<std::pair<const Key, Tp> > >
class map;

// map内部用来存储数据的是
_Rb_tree<key_type, value_type, _Select1st<value_type>, key_compare, allocator_type> _Ref;

/** 红黑树的节点信息:
树每个节点用来存储数据的是一个Value类型的变量, 对于set来说, 类型Value就是类型Key,
而对于map来说, 类型Value是std::pair<const Key, Tp>.
需要根据这个Value类型的变量来执行比较操作:
首先从这个Value类型的变量中提取出Key类型的变量(通过KeyOfValue这个函数对象),
然后再通过Compare这个函数对象来执行比较(Compare接受两个Key类型的参数, 返回值是bool类型).

红黑树节点内部还有另外四个数据成员来描述其结构:
	_Color_type _M_color;
	_Base_ptr _M_parent;
	_Base_ptr _M_left;
	_Base_ptr _M_right;
*/

// less类似这样
template <class T> struct less
{
    bool operator() (const T& x, const T& y) const
    { return x<y; }
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef bool result_type;
};

// _Identity类似这样, 简单的返回参数
template <typename T>
struct _Identity
{
    T& operator()(T& x) const
    { return x; }
    const T& operator()(const T& x) const
    { return x; }
};

// _Select1st类似这样, 返回pair参数的first
template <typename Pair>
struct _Select1st
{
    typename Pair::first_type&
    operator()(Pair& x) const
    { return x.first; }

    const typename _Pair::first_type&
    operator()(const Pair& x) const
    { return x.first; }
};

/**
sgi的红黑树内部有一个header节点, 它的parent指向root节点, left指向最左节点, right指向最右节点.
begin()返回的是header的left, end()返回的是header, 这样调用begin()只消耗常数时间.
除此之外, 还有一个数据成员存储节点个数.
再加上执行比较功能的函数对象, 就可以完整标识一棵红黑树, 所以swap函数是这样的(假定allocator相同):
*/
void swap(_Rb_tree<Key,Value,KeyOfValue,Compare,Alloc>& t)
{
    std::swap(_M_header, t._M_header);
    std::swap(_M_node_count, t._M_node_count);
    std::swap(_M_key_compare, t._M_key_compare);
}
```
##红黑树部分细节
下面展示红黑树迭代器的部分细节，和find函数的细节，其他部分暂无力理解。
```cpp
// 迭代器的自加、自减
struct _Rb_tree_base_iterator
{
    typedef _Rb_tree_node_base::_Base_ptr    _Base_ptr;
    typedef bidirectional_iterator_tag       iterator_category;
    typedef ptrdiff_t                        difference_type;

    _Base_ptr    _M_node;

    void _M_increment()
    {
        if(_M_node->_M_right != 0)
        {
            // 如果有右子树, 下一个节点将是右子树的最左
            _M_node = _M_node->_M_right;
            while(_M_node->_M_left != 0)
                _M_node = _M_node->_M_left;
        }
        else
        {
            // 如果没有右子树, 下一个节点在上面的层里
            // 如果当前节点是父节点的左孩子, 其父节点就是要找的下一个节点
            // 如果当前节点是父结点的右孩子, 一直上溯, 直到某个节点是其父结点的左孩子, 其父节点就是要找的下一个节点
            _Base_ptr y = _M_node->_M_parent;
            while(_M_node == y->_M_right)
            {
                _M_node = y;
                y = y->_M_parent;
            }
            // 当root节点并无右子树, 且欲寻找root节点的下一节点时, 下面的if会为true, 下一节点就是header节点
            // 当前节点已为最大时, 自加将使之跳到header节点, 所以end()返回的也是header节点
            if(_M_node->_M_right != y)
                _M_node = y;
        }
    }

    void _M_decrement()
    {
        if(_M_node->_M_color == _S_rb_tree_red &&
           _M_node->_M_parent->_M_parent == _M_node)
            // 当前节点为end()时, 也就是header时
            _M_node = _M_node->_M_right;
        else if(_M_node->_M_left != 0)
        {
            // 当前节点有左子树时, 上一节点即为左子树的最右
            _Base_ptr y = _M_node->_M_left;
            while(y->_M_right != 0)
                y = y->_M_right;
            _M_node = y;
        }
        else
        {
            // 当前节点无左子树时, 上一节点在上面的层里
            // 若为父节点的右孩子, 父节点即为下一节点
            // 若为父节点的左孩子, 一直上溯, 直到某个节点是其父节点的右孩子, 其父节点就是要找的下一节点
            _Base_ptr y = _M_node->_M_parent;
            while(_M_node == y->_M_left)
            {
                _M_node = y;
                y = y->_M_parent;
            }
            _M_node = y;
        }
    }
};

// 需找RB树中是否有键值为k的节点
// 为何非要找到底呢, 找到相等的返回不就完了?
// lower_bound和upper_bound的动作与此类似
// equal_range就是调用了pair<iterator,iterator>(lower_bound(k),upper_bound(k));
template <typename Key, typename Value, typename KeyOfValue,
		 typename Compare, typename Alloc>
typename _Rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::iterator
_Rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::find(const Key& k)
{
    _Link_type y = _M_header;      // Last node which is not less than k.
    _Link_type x = _M_root();      // Current node.

    while(x != 0)
        if(!_M_key_compare(_S_key(x), k)) // k小于等于节点值, 去左子树继续寻找
            y = x, x = _S_left(x); // 记下大于等于k的节点
        else // k大于节点值, 去右子树继续寻找
            x = _S_right(x);

    iterator j = iterator(y);
    // 若树为空, x等于0, j将等于end()
    // 若k小于y, 表示没有要找的节点
    return (j == end() || _M_key_compare(k, _S_key(j._M_node))) ?
           end() : j;
}
```
##hashtable，hash_set，hash_map
```cpp
// hash_set和hash_map是gcc提供的, 定义在命名空间__gnu_cxx中
// 在c++11里, unordered_map和unordered_set已经替换了hash_set和hash_map

/**
与set和map相比, hash_set和hash_map无非是把数据存储到了hashtable而不是红黑树里, 接口与set和map基本相同.
与红黑树的逻辑相似, hashtable节点中有一个Value类型的数据,
通过一个函数对象从Value数据中提取出Key类型的数据,
然后由这个Key类型的数据来确定这个节点的存储位置.
HashFcn是一个函数对象, 它接受一个Key类型的参数,
返回值类型为size_t, 就是通过它确定节点应存储在什么位置;
ExtractKey是一个函数对象, 从Value对象中提取出Key对象,
它和红黑树中KeyOfValue的作用是一样的;
EqualKey是一个函数对象, 用来判断两个Key对象是否相等.
*/
template <typename Value, typename Key, typename HashFcn,
         typename ExtractKey, typename EqualKey,
         typename Alloc = std::allocator<Value> >
class hashtable;

// hash_set
template <typename Value,
         typename HashFcn  = hash<Value>,
         typename EqualKey = equal_to<Value>,
         typename Alloc =  allocator<Value> >
class hash_set;

// hash_set内部用来存储数据的是
typedef hashtable<Value, Value, HashFcn,
                 _Identity<_Value>, EqualKey, Alloc> _Ht;

// hash_map
template <typename Key, typename Tp,
         typename HashFcn  = hash<Key>,
         typename EqualKey = equal_to<Key>,
         typename Alloc =  allocator<Tp> >
class hash_map;

// hash_map内部用来存储数据的是
typedef hashtable<pair<const Key,Tp>, Key, HashFcn,
                 _Select1st<pair<const Key,Tp> >, EqualKey, Alloc> _Ht;
```
与hashtable，hash_set，hash_multiset，hash_map，hash_multimap相关的头文件有stl_hashtable.h，stl_hash_fun.h，hash_set，hash_map。
stl_hashtable.h中定义了hashtable，stl_hash_fun.h定义了常见类型的hash函数，hash_set中定义了hash_set和hash_multiset，hash_map中定义了hash_map和hash_multimap。

stl_hash_fun.h里定义了常见整型的hash函数，只是简单的返回它们本身的值；还定义了字符串的hash函数。
除此之外的其他类型，都需要自行定义hash函数，如果对该文件提供的hash函数不满意，也可以自行定义。
```cpp
template <typename Key> struct hash { };

inline size_t __stl_hash_string(const char* s)
{
    unsigned long h = 0;
    for ( ; *s; ++s)
        h = 5*h + *s;
    return size_t(h);
}

template<> struct hash<char*>
{
    size_t operator()(const char* s) const
    { return __stl_hash_string(s); }
};

template<> struct hash<const char*>
{
    size_t operator()(const char* s) const
    { return __stl_hash_string(s); }
};

template<> struct hash<char>
{
    size_t operator()(char x) const
    { return x; }
};
template<> struct hash<unsigned char>
{
    size_t operator()(unsigned char x) const
    { return x; }
};
template<> struct hash<signed char>
{
    size_t operator()(unsigned char x) const
    { return x; }
};

template<> struct hash<short>
{
    size_t operator()(short x) const
    { return x; }
};
template<> struct hash<unsigned short>
{
    size_t operator()(unsigned short x) const
    { return x; }
};

template<> struct hash<int>
{
    size_t operator()(int x) const
    { return x; }
};
template<> struct hash<unsigned int>
{
    size_t operator()(unsigned int x) const
    { return x; }
};

template<> struct hash<long>
{
    size_t operator()(long x) const
    { return x; }
};
template<> struct hash<unsigned long>
{
    size_t operator()(unsigned long x) const
    { return x; }
};
```

	sgi以开链法完成hashtable, 称hashtable的数组为buckets, 以std::vector完成,
	buckets vector每个元素的类型是指向节点的指针.
	
	如何确定节点在哪个bucket?
	先对Key取hash后, 再执行"hash值%vector的大小"获得其bucket.
	关于buckets vector的大小
	sgi内部定义了一个拥有28个元素的整型数组, 存储了28个质数,
	这些质数逐渐呈现大约两倍的关系, 最小的质数是53,
	hashtable自动将大小提升为最接近且足够容纳所有节点的质数.
	这样设计保证了hashtable足够稀疏.
```
// 迭代器内有一个指向节点的指针, 一个指向hashtable的指针.
// 这是它的自加操作, 并不提供自减操作:
iterator& operator++()
{
    const _Node* __old = _M_cur;
    _M_cur = _M_cur->_M_next; // 链表的下一个节点
    if(!_M_cur) // 链表如已到结尾, 则要在vector里跳到下一个bucket
    {
        // 获得原节点所在的bucket
        size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
        // 当找到下一节点结束
        // 或后面的bucket空无一物时也结束, 此时_M_cur为空, 迭代器与end()相等
        while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
            _M_cur = _M_ht->_M_buckets[__bucket];
    }
    return *this;
}

// hashtable理解起来并不困难, 我也懒得去标注源代码了.
```
hash_set和hash_map缺省使用大小为100的表格，hashtable将调整为193。