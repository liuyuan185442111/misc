map是映射的意思，存储&lt;key，value>对，在map里，这个数据对整体作为value。
map和set类似，所以不多解释了。
```cpp
template <typename Key, typename Tp, typename Compare = std::less<Key>,
         typename Alloc = std::allocator<std::pair<const Key, Tp> > >
class map
{
public:
    typedef Key                                      key_type;
    typedef Tp                                       mapped_type;
    typedef std::pair<const Key, Tp>                 value_type;
    typedef Compare                                  key_compare;
    typedef Alloc                                    allocator_type;

public:
    // 对value_type进行比较的对象, 实际上比较规则和Key的比较规则相同
    // 没有public constructor, 只有map的成员函数能生成其实例
    class value_compare : public std::binary_function<value_type, value_type, bool>
    {
        friend class map<Key, Tp, Compare, Alloc>;
    protected:
        Compare comp;
        value_compare(Compare c) : comp(c) { }
    public:
        bool operator()(const value_type& x, const value_type& y) const
        {
            return comp(x.first, y.first);
        }
    };

private:
    // _Select1st是一个函数对象, 调用它将返回pair的第一个元素
    typedef _Rb_tree<key_type, value_type, _Select1st<value_type>,
            key_compare, allocator_type> _Rep_type;
    // The actual tree structure.
    _Rep_type _M_t;

public:
    // 可能与ISO不同, 但功能等价
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename _Rep_type::iterator               iterator;
    typedef typename _Rep_type::const_iterator         const_iterator;
    typedef typename _Rep_type::reverse_iterator       reverse_iterator;
    typedef typename _Rep_type::const_reverse_iterator const_reverse_iterator;
    typedef typename _Rep_type::size_type              size_type;
    typedef typename _Rep_type::difference_type        difference_type;


public:
    // 构造
    explicit map(const key_compare& comp = key_compare(),
                 const allocator_type& alloc = allocator_type());

    map(const map& x) : _M_t(x._M_t) { }

    // 创建一个包含[first,last)元素拷贝的map.
    // 如果序列有序, O(N)复杂度;否则, O(NlogN)复杂度.N=distance(first,last)
    template <class InputIterator>
    map(InputIterator first, InputIterator last,
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type());


    // 赋值
    map& operator=(const map& x)
    {
        _M_t = x._M_t;
        return *this;
    }


    // get
    allocator_type get_allocator() const
    {
        return allocator_type(_M_t.get_allocator());
    }

    key_compare key_comp() const
    {
        return _M_t.key_comp();
    }

    value_compare value_comp() const
    {
        return value_compare(_M_t.key_comp());
    }


    // Iterators
    iterator begin()
    {
        return _M_t.begin();
    }

    const_iterator begin() const
    {
        return _M_t.begin();
    }

    iterator end()
    {
        return _M_t.end();
    }

    const_iterator end() const
    {
        return _M_t.end();
    }

    reverse_iterator rbegin()
    {
        return _M_t.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return _M_t.rbegin();
    }

    reverse_iterator rend()
    {
        return _M_t.rend();
    }

    const_reverse_iterator rend() const
    {
        return _M_t.rend();
    }


    // Capacity
    bool empty() const
    {
        return _M_t.empty();
    }

    size_type size() const
    {
        return _M_t.size();
    }

    size_type max_size() const
    {
        return _M_t.max_size();
    }


    // Element access
    // 如果map中不存在k, 就执行插入; 如果不存在, 找到正确位置. 返回mapped_type
    mapped_type& operator[](const key_type& k)
    {
        iterator i = lower_bound(k);
        // i->first is greater than or equivalent to k.
        if (i == end() || key_comp()(k, (*i).first))
            i = insert(i, value_type(k, mapped_type()));
        return (*i).second;
    }


    // modifiers
    /**
    此函数尝试将一个(key, value)对插入map.
    由于key值唯一, 只有当key值不存在于map中时, 才会执行插入动作.
    插入成功与否存储于返回pair的第二个元素里;
    如果成功插入, pair第一个元素存储指向插入元素的迭代器, 否则, 存储指向已存在元素的迭代器.
    需要对数时间.
    */
    std::pair<iterator, bool>
    insert(const value_type& __x)
    {
        return _M_t._M_insert_unique(__x);
    }
    
    //c++98里, 如果position指向被插入元素之前, 函数会优化插入时间
    //c++11里, 如果position指向被插入元素之后, 函数会优化插入时间
    iterator insert(iterator position, const value_type& val);

    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        _M_t._M_insert_unique(first, last);
    }

    iterator  erase(const_iterator position);

    /**
    返回擦除元素的个数
    *  Note that this function only erases the element, and that if
    *  the element is itself a pointer, the pointed-to memory is not touched
    *  in any way.  Managing the pointer is the user's responsibility.
    */
    size_type erase(const key_type& __x)
    {
        return _M_t.erase(__x);
    }
    
    void erase(iterator first, iterator last)
    {
        _M_t.erase(first, last);
    }

    void swap(map& x)
    {
        _M_t.swap(x._M_t);
    }

    void clear()
    {
        _M_t.clear();
    }


    // operations
    iterator find(const key_type& x)
    {
        return _M_t.find(x);
    }

    const_iterator find(const key_type& x) const
    {
        return _M_t.find(x);
    }

    size_type count(const key_type& x) const
    {
        return _M_t.find(x) == _M_t.end() ? 0 : 1;
    }

    // 返回指向第一个equal to or greater than key的元素的迭代器或end().
    iterator lower_bound(const key_type& x)
    {
        return _M_t.lower_bound(x);
    }

    const_iterator lower_bound(const key_type& x) const
    {
        return _M_t.lower_bound(x);
    }

    // 返回指向第一个greater than key的元素的迭代器或end().
    iterator upper_bound(const key_type& x)
    {
        return _M_t.upper_bound(x);
    }

    const_iterator upper_bound(const key_type& x) const
    {
        return _M_t.upper_bound(x);
    }

    /**
    Finds a subsequence matching given key.
    返回可能符合要求的子序列的迭代器对.
    此函数等价于std::make_pair(c.lower_bound(val),  c.upper_bound(val)),
    但是比单独调用快.
    此函数对multimap比较有意义.
    */
    std::pair<iterator, iterator>
    equal_range(const key_type& x)
    {
        return _M_t.equal_range(x);
    }

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& x) const
    {
        return _M_t.equal_range(x);
    }
    /**
    以上两个函数看起来调用同样的函数?
    仅返回值不同, 但是返回值不是函数标的一部分,
    然而, 后者有个const, 看红黑树里的函数声明:
    pair<iterator, iterator>
    equal_range(const key_type& k);
    pair<const_iterator, const_iterator>
    equal_range(const key_type& k) const;
    */
}
```
