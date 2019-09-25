	template < class T, class Alloc = allocator<T> > class list;
内部实现是一个双向循环链表，为符合前闭后开的特点，添加了一个空节点，list内部存储的即指向此空节点的指针，仅凭此指针便可标识整个链表。
begin()获得的是此指针的next，end()获得的是此指针。
下面是list的接口：
```cpp
// 构造、析构、赋值、获得allocator
explicit list(const allocator_type& alloc = allocator_type());
explicit list(size_type n, const value_type& val = value_type(),
				const allocator_type& alloc = allocator_type());
template <class InputIterator>
list(InputIterator first, InputIterator last,
		const allocator_type& alloc = allocator_type());
list(const list& x);
~list();
list& operator=(const list& x);
allocator_type get_allocator() const;

// 迭代器
iterator begin();
const_iterator begin() const;
terator end();
const_iterator end() const;
reverse_iterator rbegin();
const_reverse_iterator rbegin() const;
reverse_iterator rend();
const_reverse_iterator rend() const;

// 容量
bool empty() const;
size_type size() const;
size_type max_size() const;

// 成员访问
// 即*begin()
reference front();
const_reference front() const;
// 即*(--end())
reference back();
const_reference back() const;

// 修改
template <class InputIterator>
void assign(InputIterator first, InputIterator last);
void assign(size_type n, const value_type& val);
// 即insert(begin(), val), 新插入的节点是新的头节点
void push_front(const value_type& val);
// 即erase(begin())
void pop_front();
// 即insert(end(), val)
void push_back(const value_type& val);
// 即erase(--end())
void pop_back();
// 都是在position之前插入
// 返回指向插入节点的迭代器
iterator insert(iterator position, const value_type& val);
void insert(iterator position, size_type n, const value_type& val);
template <class InputIterator>
void insert(iterator position, InputIterator first, InputIterator last);
// 返回指向position后面节点的迭代器
iterator erase(iterator position);
// 实际是调用上面的erase(), 返回指向last后面节点的迭代器
iterator erase(iterator first, iterator last);
// 实际是交换指向空节点的指针
void swap(list& x);
void resize(size_type n, value_type val = value_type());
// erase所有节点
void clear();

// 操作
// 将x拼接到position之前
void splice(iterator position, list& x);
// 将i指向的节点拼接到position之前
void splice(iterator position, list& x, iterator i);	
// 将[first,last)拼接到position之前
void splice(iterator position, list& x, iterator first, iterator last);
// erase掉所有值为val的节点
void remove(const value_type& val);
// erase掉所有pred(*iterator)为真的节点
template <class Predicate>
void remove_if(Predicate pred);
// 将所有相同的相邻节点减少为1个, 保留第一个出现的, erase掉其他的
void unique();
// 节点相同的判定条件为binary_pred(*cur, *next)为真
template <class BinaryPredicate>
void unique(BinaryPredicate binary_pred);
// 需要保证*this和x都为升序, merge完毕, x为空, *this升序排序
void merge(list& x);
// 当comp(x,y)表示x<y时, 最终list是升序排列
template <class Compare>
void merge(list& x, Compare comp);
// 调用merge()进行排序
void sort();
// comp是merge()的排序规则
template <class Compare>
void sort(Compare comp);
  
// 关系操作符和swap
template <class T, class Alloc>
void swap(list<T,Alloc>& x, list<T,Alloc>& y);
template <class T, class Alloc>
bool operator==(const list<T,Alloc>& lhs, const list<T,Alloc>& rhs);
template <class T, class Alloc>
bool operator!=(const list<T,Alloc>& lhs, const list<T,Alloc>& rhs);
template <class T, class Alloc>
bool operator<(const list<T,Alloc>& lhs, const list<T,Alloc>& rhs);
template <class T, class Alloc>
bool operator<=(const list<T,Alloc>& lhs, const list<T,Alloc>& rhs);
template <class T, class Alloc>
bool operator>(const list<T,Alloc>& lhs, const list<T,Alloc>& rhs);
template <class T, class Alloc>
bool operator>=(const list<T,Alloc>& lhs, const list<T,Alloc>& rhs);
```
并没有看懂它的排序算法（这是gcc 4.7.1里的）：
```cpp
template <typename _Tp, typename _Alloc>
void list<_Tp, _Alloc>::sort()
{
    // Do nothing if the list has length 0 or 1.
    if (this->_M_impl._M_node._M_next != &this->_M_impl._M_node
            && this->_M_impl._M_node._M_next->_M_next != &this->_M_impl._M_node)
    {
        list __carry;
        list __tmp[64];
        list * __fill = &__tmp[0];
        list * __counter;

        do
        {
            __carry.splice(__carry.begin(), *this, begin());

            for(__counter = &__tmp[0];
                    __counter != __fill && !__counter->empty();
                    ++__counter)
            {
                __counter->merge(__carry);
                __carry.swap(*__counter);
            }
            __carry.swap(*__counter);
            if (__counter == __fill)
                ++__fill;
        }
        while ( !empty() );

        for (__counter = &__tmp[1]; __counter != __fill; ++__counter)
            __counter->merge(*(__counter - 1));
        swap( *(__fill - 1) );
    }
}
```
我尝试改成普通函数来理解，还是没懂：
```cpp
#include <iostream>
#include <list>
using namespace std;

void show(list<int>&l)
{
    cout << ':';
    list<int>::iterator i;
    for(i=l.begin(); i!=l.end(); ++i)
    {
        cout << *i << ' ';
    }
    cout << endl;
}
void sortx(list<int> &l);
int main()
{
    list<int> l;
    l.push_back(3);
    l.push_back(2);
    l.push_back(5);
    l.push_back(1);
    l.push_back(4);
    l.push_back(6);
    l.push_back(7);
    l.push_back(8);
    show(l);
    cout << endl;

    sortx(l);
    //l.sort();
    //show(l);

    return 0;
}

// 神奇的算法
void sortx(list<int> &l)
{
    list<int> carry;
    list<int> tmp[64];
    list<int> * fill = &tmp[0];
    list<int> * counter;

    do
    {
        carry.splice(carry.begin(), l, l.begin());

        // counter到达目前最大值或counter为空时结束
        for(counter = &tmp[0]; counter != fill && !counter->empty(); ++counter)
        {
            counter->merge(carry); // carry里的数据merge到counter里, carry变为空
            carry.swap(*counter);
        }
        carry.swap(*counter);
        if (counter == fill)
            ++fill;

        //show(carry); // 总是空的!
        // 依次存1,2,4,8...个数据, 满了推到上层
        cout << "---------\n";
        show(tmp[0]);
        show(tmp[1]);
        show(tmp[2]);
        show(tmp[3]);
        getchar();
    }
    while ( !l.empty() );

    for (counter = &tmp[1]; counter != fill; ++counter)
        counter->merge(*(counter - 1));
    l.swap( *(fill - 1) );
}
```