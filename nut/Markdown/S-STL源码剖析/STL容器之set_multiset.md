set/multiset会根据待定的排序准则，自动将元素排序。两者不同在于前者不允许元素重复，而后者允许。
##关于set
对于set，元素的值标识这个元素。每个元素必须是唯一的，且一旦存入set就不可修改(the elements are always const)，但是可以进行插入和删除操作。

访问单个元素，set通常比unordered_set慢，但是set允许按顺序直接插入。
set通常用二叉搜索树实现（sgi用红黑树实现）。

**关于Compare**
Compare可以是一个函数指针或函数对象，Compare(a,b) shall return true if a is considered to go before b in the strict weak ordering。

**关于strict weak ordering**
The STL algorithms for stable_sort( ) and sort( ) require the binary predicate to be strict weak ordering.
For example:
· Strict: `pred (X, X) is always false`.
· Weak: `If !pred(X,Y) && !pred(Y,X), then X==Y`.
· Ordering:` If pred(X,Y) && pred(Y,Z), then pred(X,Z)`.

set的迭代器是双向迭代器。

set的接口如下，基本是转调了红黑树的接口而已：
```cpp
template < typename T, typename Compare = less<T>,
         typename Alloc = allocator<T> >
class set
{
public:
    typedef T       key_type;
    typedef T       value_type;
    typedef Compare key_compare;
    typedef Compare value_compare;
    typedef Alloc   allocator_type;

    typedef typename allocator_type::pointer             pointer;
    typedef typename allocator_type::const_pointer       const_pointer;
    typedef typename allocator_type::reference           reference;
    typedef typename allocator_type::const_reference     const_reference;

private:
    // 红黑树
    typedef _Rb_tree<key_type, value_type, _Identity<key_type>, key_compare, allocator_type> _Rep_type;

public:
    typedef typename _Rep_type::const_iterator            iterator;
    typedef typename _Rep_type::const_iterator            const_iterator;
    typedef typename _Rep_type::const_reverse_iterator    reverse_iterator;
    typedef typename _Rep_type::const_reverse_iterator const_reverse_iterator;
    typedef typename _Rep_type::size_type                 size_type;
    typedef typename _Rep_type::difference_type           difference_type;

    // 构造
    explicit set(const key_compare& comp = key_compare(),
                 const allocator_type& alloc = allocator_type());

    template <typename InputIterator>
    set(InputIterator first, InputIterator last,
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type());

    set (const set& x);

    // 析构
    ~set();

    // 赋值
    set& operator=(const set& x);

    // 迭代器
    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;

    reverse_iterator rend();
    const_reverse_iterator rend() const;

    // Capacity
    bool empty() const;
    size_type size() const;
    size_type max_size() const;

    // Modifiers
    /**
    由于set的元素是唯一的, 如果val已存在于set中, 插入操作则会失败;
    插入操作的状态存入返回值的第二个元素里,
    如果插入失败, 返回pair的第一个元素存储的是指向此元素的迭代器,
    如果插入成功, 其存储的是指向新插入元素的迭代器.
    */
    pair<iterator,bool> insert(const value_type& val);

    /**
    此函数不关心插入是否成功, 返回值和上个函数返回pair的第一个元素相同.
    第一个参数仅是一个hint, 它可能会提高插入的效率.
    A bad hint would cause no gains in efficiency.
    Insertion requires logarithmic time (if the hint is not taken).
    */
    iterator insert(iterator position, const value_type& val);

    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last);

    void erase(iterator position);
    size_type erase(const value_type& val);
    void erase(iterator first, iterator last);

    void swap (set& x);
    void clear();

    // get操作
    key_compare key_comp() const;
    value_compare value_comp() const;
    allocator_type get_allocator() const;

    // Operations
    // 没有找到将返回end()
    iterator find(const value_type& val) const;
    // 返回0或1
    size_type count(const value_type& val) const;

    /**
    Return iterator to lower bound
    返回指向第一个满足not considered to go before val的元素的迭代器
    即key_compare(element,val)返回false, 也就是element>=val
    */
    iterator lower_bound(const value_type& val) const;

    // 返回指向第一个满足considered to go after val的元素的迭代器
    // 当set包含一个等于val的元素时, lower_bound(val)返回指向这个元素的迭代器,
    // 而upper_bound(val)返回指向下个元素的迭代器
    iterator upper_bound(const value_type& val) const;

    /**
    返回与val相等的所有元素的边界
    因为set里所有元素都是唯一的, 所以此序列最多包含一个元素
    如果没有符合的值, 两个迭代器都指向val的下一个元素, 此元素被认为go after val
    如果找到, 第一个迭代器指向val, 第二个迭代器指向下一个元素
    */
    pair<iterator,iterator> equal_range(const value_type& val) const;
}
```
##关于multiset
multiset的特性以及用法和set完全相同，唯一的差别在于它允许键值重复。另外还要注意以下几点：
multiset::find()仅返回第一个查找到的元素，如要获得所有元素序列，可使用multiset::equal_range()。
size_type multiset::erase (const value_type& val);会erase掉所有与val相等的元素。
对于插入操作，不能保证相等元素的相对顺序。
##例子1
cplusplus.com上自定义Comparator的例子：
```cpp
// http://www.cplusplus.com/reference/set/set/set/
// 其实是关于构造函数的例子
// constructing sets
#include <iostream>
#include <set>

bool fncomp(int lhs, int rhs) {return lhs<rhs;}

struct classcomp {
  bool operator() (const int& lhs, const int& rhs) const
  {return lhs<rhs;}
};

int main()
{
  std::set<int> first;                           // empty set of ints

  int myints[] = {10,20,30,40,50};
  std::set<int> second(myints,myints+5);        // range

  std::set<int> third(second);                  // a copy of second

  std::set<int> fourth(second.begin(), second.end());  // iterator ctor.

  std::set<int,classcomp> fifth;                 // class as Compare

  bool (*fn_pt)(int,int) = fncomp;
  std::set<int,bool(*)(int,int)> sixth(fn_pt);  // function pointer as Compare

  return 0;
}
```
本来我是不准备展示这个例子的，但是在思考下一个例子的时候，发现还是有必要对class和function pointer作为Comparator的不同做些说明。
可以看到，当用class作为Comparator时，构造函数并不需要参数，因为有默认参数key_compare()，这样OK；但是当用function pointer作为Comparator的时候就不OK了，虽然编译不会出错，但运行起来肯定会出错！

那为什么会编译通过？
考虑到

    int m = int();
    cout << m << endl;
会输出0，似乎c++对基本数据类型进行了“类”化，int()相当于用0初始化m，bool(*)(int,int)()就是用0初始化这个函数指针类型。

所以，当使用函数指针类型来做为Comparator时一定要用实际的函数指针来初始化。
##例子2
当set用int作为参数时，key就是int类型，insert一个元素就是拷贝一个int，也没啥开销。但当元素类型是一个类时，insert一个元素就会调用拷贝构造函数，这开销就可能很大了。
**可以用指向类的指针作为set的参数！只要自定义Comparator就好了。**
下面就是一个例子：
```cpp
#include <iostream>
#include <set>
#include <cstdlib>
using namespace std;

struct Cont
{
    Cont(int m=0,int n=0):a(m),b(n) {}
    int a,b;
    void show() { cout << a+b << endl; }
};

bool com(Cont *a, Cont *b)
{
    return a->a+a->b < b->a+b->b;
}

int main ()
{
    set<Cont*,bool (*)(Cont*,Cont*)> myset(com);
    Cont *p = NULL;
    for(int i=0; i<5; ++i)
    {
        p = new Cont(rand(),rand());
        myset.insert(p);
    }

    set<Cont*,bool (*)(Cont*,Cont*)>::iterator ite;
    for(ite=myset.begin(); ite!=myset.end(); ++ite)
    {
        (*ite)->show();
        delete *ite;
    }

    return 0;
}
```
当然这里我们要负责处理这些指针，比如new和delete。
既然可以用指针，那么可以用引用吗？答案是不行。
set用到的很多类里都有类似这样的定义：typedef  T* pointer;，然而定义一个指向引用的指针是非法的：typedef int&*pointer;（错误！）。
##参考
http://www.cplusplus.com/reference/set/
STL源码剖析