**在c++11里auto_ptr被声明为过时的。**
智能指针在std_memory.h中实现，下面是它的源码（稍有改动）：
```
template <class _Tp1>
struct auto_ptr_ref
{
    _Tp1* _M_ptr;
    auto_ptr_ref(_Tp1* __p) : _M_ptr(__p) {}
};

template <class _Tp>
class auto_ptr
{
private:
    _Tp *_M_ptr;

public:
    typedef _Tp element_type;

    auto_ptr(_Tp *p = 0) throw() : _M_ptr(p) {}

    // 复制构造函数和赋值函数
    auto_ptr(auto_ptr &a) throw() : _M_ptr(a.release()) {}
    auto_ptr &operator=(auto_ptr &__a) throw()
    {
        reset(__a.release());
        return *this;
    }

    // 泛化复制和赋值
    template <class _Tp1>
    auto_ptr(auto_ptr<_Tp1> &__a) throw()
        : _M_ptr(__a.release()) {}
    template <class _Tp1>
    auto_ptr &operator=(auto_ptr<_Tp1> &__a) throw()
    {
        reset(__a.release());
        return *this;
    }

    // 类型转换运算符
    template <class _Tp1>
    operator auto_ptr<_Tp1>() throw()
    {
        return auto_ptr<_Tp1>(this->release());
    }

    ~auto_ptr()
    {
        delete _M_ptr;
    }

    _Tp &operator*() const throw()
    {
        return *_M_ptr;
    }
    _Tp *operator->() const throw()
    {
        return _M_ptr;
    }
    _Tp *get() const throw()
    {
        return _M_ptr;
    }

    // 将指针清零并返回清零前的指针
    _Tp *release() throw()
    {
        _Tp *__tmp = _M_ptr;
        _M_ptr = 0;
        return __tmp;
    }
    // 将_M_ptr清零后指向新的内容
    void reset(_Tp* __p = 0) throw()
    {
        if (__p != _M_ptr)
        {
            delete _M_ptr;
            _M_ptr = __p;
        }
    }

// 以下存在意义在于接受右值参数, 详见于对应文档
public:
    template <class _Tp1>
    operator auto_ptr_ref<_Tp1>() throw()
    {
        return auto_ptr_ref<_Tp>(this->release());
    }

    auto_ptr(auto_ptr_ref<_Tp> __ref) throw()
        : _M_ptr(__ref._M_ptr) {}
    auto_ptr &operator=(auto_ptr_ref<_Tp> __ref) throw()
    {
        if (__ref._M_ptr != this->get())
        {
            delete _M_ptr;
            _M_ptr = __ref._M_ptr;
        }
        return *this;
    }
};
```
##原理
auto_ptr的拷贝构造函数和赋值操作符，与一般类的做法不太相同，它们的参数都是auto_ptr&，而不是auto_ptr const &。一般来说，类的拷贝构造函数和赋值操作符的参数都是const &。但是auto_ptr的做法也是合理的：**确保拥有权能够转移**。
auto_ptr强调对资源的**拥有权（ownership）**。也就是说，auto_ptr是“它所指对象”的拥有者，一个对象只能属于一个拥有者。
为了保证auto_ptr的拥有权唯一，auto_ptr的拷贝构造函数和赋值操作符做了这样一件事情：**移除另一个auto_ptr的拥有权**。
如果auto_ptr的拷贝构造函数和赋值操作符的参数是auto_ptr const &，那么实参的拥有权将不能转移。因为转移拥有权需要修改auto_ptr的成员变量，而实参确是一个const对象，不允许修改。
##问题
但是呢，这样会带来一个问题。
假如写出这样的代码：
```
#include <iostream>
#include <memory>
using namespace std;

int main(int argc, char **argv) {
	auto_ptr<int> ptr1(auto_ptr<int>(new int(1))); // 使用临时对象进行拷贝构造
	auto_ptr<int> ptr2;
	ptr2 = (auto_ptr<int>(new int(1))); // 使用临时对象进行赋值
}
```
假设没有定义auto_ptr_ref类及相关的函数，那么这段代码将不能通过编译。主要的原因是，拷贝构造函数及赋值操作符的参数：`auto_ptr<int>(new int(1))`是**临时对象**。临时对象属于典型的**右值**，而**<font color=#DC143C>非const &是不能指向右值的</font>**。（参见More Effective C++，Item 19）。比如：

	int &a = 5; // 错误
	const int &b = 5; // 可以, 仅语法通过, 实际没什么意义, const int b = 5;效果是一样的
auto_ptr的拷贝构造函数及赋值操作符的参数类型恰恰是auto_ptr&，而非const auto_ptr &。
同理，下面的两段代码，也不会通过编译：
```cpp
#include <iostream>
#include <memory>
using namespace std;
auto_ptr<int> f() {
	return auto_ptr<int>(new int(1)); // 这里也使用临时对象进行拷贝构造
}
int main() {
	auto_ptr<int> ptr3(f()); // 使用临时对象进行拷贝构造
	auto_ptr<int> ptr4;
	ptr4 = f(); // 使用临时对象进行赋值  
}
```
普通类不会遇到这个问题，是因为他们的拷贝构造函数及赋值操作符（不管是用户定义还是编译器生成的版本），参数都是const &。
##auto_ptr_ref之原理
下面的构造函数，是可以接受auto_ptr临时对象的。

	auto_ptr(auto_ptr __a) throw() : _M_ptr(__a.release()) { }
但上述构造函数不能通过编译，因为它会循环调用自己（该构造函数会调用auto_ptr的拷贝构造函数）。我们稍作修改：

	auto_ptr(auto_ptr_ref<element_type> __ref) throw() // element_type就是auto_ptr的模板参数
	: _M_ptr(__ref._M_ptr) {}
该版本的构造函数，可以接受auto_ptr_ref的临时对象。如果auto_ptr可以隐式转换到auto_ptr_ref，那么我们就能够用auto_ptr临时对象来调用该构造函数。这个隐式转换不难实现：

	template<typename _Tp1>
	operator auto_ptr_ref<_Tp1>() throw()
	{ return auto_ptr_ref<_Tp1>(this->release()); }
好像auto_ptr_ref赋予了auto_ptr“引用”的语义一样。
##用mutable突破限制
上面问题的根源在于**非const &是不能指向右值的**，那么我们就让复制构造和赋值函数的形参变成const &。
因为形参auto_ptr const& a会调用release()，所以release()必须也变为const，但是release()还需要修改私有成员_M_ptr的值。此时，mutable修饰符出场，将_M_ptr限定为mutable的，问题解决。
实现如下（顺带测试了一下）：
```
#include <iostream>
template <typename T>
class auto_ptr
{
private:
    mutable T *M_ptr;

public:
    auto_ptr(T *p = 0) throw() : M_ptr(p) {}
    auto_ptr(auto_ptr const &a) throw() : M_ptr(a.release()) {}
    auto_ptr &operator=(auto_ptr const &a) throw()
    {
        reset(a.release());
        return *this;
    }
    ~auto_ptr()
    {
        delete M_ptr;
    }

    T &operator*() const throw()
    {
        return *M_ptr;
    }
    T *operator->() const throw()
    {
        return M_ptr;
    }
    T *get() const throw()
    {
        return M_ptr;
    }

    // 唯有以下两个函数会对M_ptr进行修改
    T *release() const throw()
    {
        T *tmp = M_ptr;
        M_ptr = 0;
        return tmp;
    }
    void reset(T *p = 0) throw()
    {
        if (p != M_ptr)
        {
            delete M_ptr;
            M_ptr = p;
        }
    }

    // 泛化复制和赋值
    template <class T1>
    auto_ptr(auto_ptr<T1> &a) throw() : M_ptr(a.release()) {}
    template <class T1>
    auto_ptr &operator=(auto_ptr<T1> &a) throw()
    {
        reset(a.release());
        return *this;
    }
    // 类型转换运算符
    template <class T1>
    operator auto_ptr<T1>() throw()
    {
        return auto_ptr<T1>(this->release());
    }
};

int main()
{
    auto_ptr<int> ptr1(auto_ptr<int>(new int(1)));
    auto_ptr<int> ptr2;
    ptr2 = (auto_ptr<int>(new int(1)));
    std::cout << *ptr1 << *ptr2;
    return 0;
}
```
需要注意的是，`mutable T *M_ptr;`可以写成`T mutable *M_ptr;`，但不能写成`T * mutable M_ptr;`，虽然从意义上来说mutable修饰的确实是M_ptr。
mutable只能用于修饰类的非静态数据成员，它的存在就是为了突破const的限制，const auto_ptr的成员M_ptr是不可变的，但*M_ptr是可变的，既然mutable的功能只是为了突破const的限制，必然是让M_ptr变得可变。mutable的功能单一，放在语句的最前边也就行了。
虽然mutable和const是一对反义词，但从语法上来说，volatile和const比较像。
##参考
为什么需要auto_ptr_ref
http://www.iteye.com/topic/746062