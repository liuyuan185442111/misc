##异常分类
语言本身或标准程序库所抛出的所有异常，都派生自基类exception。所有这些标准异常可分为三组：（1）语言本身支持的异常；（2）逻辑错误；（3）运行时错误。
用户程序可以从第一类异常派生出自定义异常；标准库和用户程序都可以抛出后两种异常。
![标准异常](http://img.blog.csdn.net/20150527091355150)
(图片来自参考2)

	基础类别exception和bad_exception定义于<exception>
	bad_alloc定义于<new>
	bad_cast和bad_typeid定义于<typeinfo>
	ios_base::failure定义于<ios>
	其他异常类别定义于<stdexcept>
###1. 语言本身支持的异常
此类异常用以支撑某些语言特性。
(1) new操作失败，会抛出bad_alloc异常（`<new>`中定义，new的nothrow版本不抛出异常）。
(2) 执行期间，当一个作用于reference身上的“动态型别转换操作”失败时，dynamic_cast会抛出bad_cast异常。
(3) 执行期型别辨识（RTTI）过程中，如果交给typeid的参数为零或空指针，typeid操作符会抛出bad_typeid异常。
(4) 如果发生非预期的异常（函数抛出异常规格（exception specification）以外的异常），程序会自动调用unexpected()，如果用set_unexpected自定义了unexpected函数，自定义unexpected函数中有`throw;`或抛出了不在异常规格中的其他异常，c++运行时环境将自动抛出bad_exception。。
If a function with bad_exception listed in its dynamic-exception-specifier throws an exception not listed in it and unexpected rethrows it (or throws any other exception also not in the dynamic-exception-specifier), a bad_exception is automatically thrown.（来自参考3）

这四异常都有三个函数：一个是构造函数，不带参数；一个是析构函数，是virtual的；第三个是virtual const char* what() throw()。这三个函数都不会再抛出异常。
(5) 还有一个异常ios_base::failure，它定义在ios_base类内部，当数据流由于错误或者到达文件末尾而发生状态改变时，就可能抛出这个异常。
###2. 逻辑错误
这类异常总是派生自logic_error。
(1) domain_error指出专业领域范畴内的错误。
(2) invalid_argument表示无效参数，例如将bitset(array of bits)以char而非0或1进行初始化。
(3) length_error指出某个行为“可能超越了最大极限”，例如对某个字符串附加太多字符。
(4) out_of_range指出参数值“不在预期范围内”，例如在处理容器或string中采用一个错误索引。
###3. 运行时错误
这类异常派生自runtime_error，用来指出“不在程序范围内，且不容易回避”的事件。
(1) range_error指出内部计算时发生区间错误。
(2) overflow_error指出算术运算发生上溢位。
(3) underflow_error指出算术运算发生下溢位。
##源码
```cpp
//std_stdexcept.h
class logic_error : public exception
{
    string _M_msg;
public:
    explicit logic_error(const string&  __arg);
    virtual ~logic_error() throw();
    virtual const char* what() const throw();
};

class domain_error : public logic_error
{
public:
    explicit domain_error(const string&  __arg);
};

class invalid_argument : public logic_error
{
public:
    explicit invalid_argument(const string&  __arg);
};

class length_error : public logic_error
{
public:
    explicit length_error(const string&  __arg);
};

class out_of_range : public logic_error
{
public:
    explicit out_of_range(const string&  __arg);
};


class runtime_error : public exception
{
    string _M_msg;
public:
    explicit runtime_error(const string&  __arg);
    virtual ~runtime_error() throw();
    virtual const char* what() const throw();
};

class range_error : public runtime_error
{
public:
    explicit range_error(const string&  __arg);
};

class overflow_error : public runtime_error
{
public:
    explicit overflow_error(const string&  __arg);
};

class underflow_error : public runtime_error
{
public:
    explicit underflow_error(const string&  __arg);
};
```
```cpp
//exception（稍有改动）
// 标准库可能抛出的所有异常的基类
class exception
{
public:
    exception() throw() { }
    virtual ~exception() throw();
    // 返回描述当前错误的字符串, 派生类应覆写此函数
    virtual const char* what() const throw();
};

class bad_exception : public exception
{
public:
    bad_exception() throw() { }
    virtual ~bad_exception() throw();
    virtual const char* what() const throw();
};
// 参考terminate的说明
typedef void (*unexpected_handler)();
unexpected_handler set_unexpected(unexpected_handler) throw();
// 默认调用terminate, 但可以通过set_unexpected()更改其行为
void unexpected();

/**
* 当一个异常没有catch块捕获时(一层一层throw, throw到main也没有catch处理它),
* 或出现无法处理的情况, 系统将自动调用终结函数,
* 系统提供了一个默认的终结函数terminate(), terminate()会调用abort()退出程序,
* 可以通过set_terminate()来设置新的终结函数, 终结函数必须是terminate_handler类型的,
* terminate()也可以被用户调用.
*/
typedef void (*terminate_handler)();
terminate_handler set_terminate(terminate_handler) throw();
void terminate() throw();

// 当一个异常已经被抛出, 但在相应的处理程序中异常的初始化还未完成时,
// 此函数返回true; 其他情况返回false.
// 当此函数返回true时抛出其他异常将导致terminate()被调用.
bool uncaught_exception() throw();
```
##类型匹配
当catch语句针对异常进行类型匹配的时候，有两种类型转换可能发生：
1. 基于继承的类型转换。一个被声明用于捕获基类异常的catch语句也可以处理异常基类的派生类。
2. 从一种指针类型转换到无类型指针，一个针对const void *或void *的catch语句可以捕获任何异常类型的指针。
##拷贝
c++以拷贝的方式抛出异常。
用`throw;`重新抛出异常的时候抛出的是最开始的那个拷贝。
这是有意义的，很多时候我们抛出的是一个局部变量，所以需要为其复制一份，并使之有效期持续到该异常完全被处理完毕。

看一个例子：
```
#include <iostream>
using namespace std;

class A
{
public:
    A(){cout << "in A constructor\n";}
    ~A(){cout << "in A destructor\n";}
};

class B : public  A
{
public:
    B(){cout << "in B constructor\n";}
    ~B(){cout << "in B destructor\n";}
};

B b;

void fun()
{
    try
    {
        throw b;
    }
    catch(A a)//发生了截断,用引用就好一些
    {
        cout << "catch A\n";
        throw;
    }
}
int main()
{
    try
    {
        fun();
    }
    catch(...)
    {
        cout << "catch all\n";
    }

    return 0;
}
```
输出是：

	in A constructor
	in B constructor
	catch A
	in A destructor
	catch all
	in B destructor
	in A destructor
	in B destructor
	in A destructor
`throw b;`扔出的是b的一个复制品，而不是b本身。
`throw;`扔出的是b的那个复制品，而不是a。
输出的前两行是语句`B b;`调用B的构造函数，最后四行是b的复制品的析构函数和b的析构函数。

只有在一个catch子句评估完毕并且知道它不会再抛出exception之后，真正的exception object(也就是b的那个复制品)才会被摧毁。
在整个异常传递过程中，只会产生一个exception object。
##异常规格
参考3提供了一个例子：
```cpp
// bad_exception example
#include <iostream>       // std::cerr
#include <exception>      // std::bad_exception, std::set_unexpected

void myunexpected() {
  std::cerr << "unexpected handler called\n";
  throw;
}

void myfunction() throw (int, std::bad_exception) {
  throw 'x'; // throws char (not in exception-specification)
}

int main(void) {
  std::set_unexpected(myunexpected);
  try {
    myfunction();
  }
  catch(int) { std::cerr << "caught int\n"; }
  catch(std::bad_exception be) { std::cerr << "caught bad_exception\n"; }
  catch(...) { std::cerr << "caught some other exception\n"; }
  return 0;
}
```
输出是：
unexpected handler called
caught bad_exception
##析构函数与异常
在异常传递过程中执行到堆栈开解（stack-unwinding）部分，异常机制会销毁所有局部变量。如果这些局部变量在销毁的过程中又抛出了异常，会导致到调用terminate函数。（函数uncaught_exception可以识别这种情况。）
看这样的例子：
```
struct B
{
    ~B() { cout << "destruct B\n"; }
};

struct C
{
    B b;
    ~C()
    {
        cout << "destruct C\n";
        cout << "uncaught_exception? " << boolalpha << uncaught_exception() << endl;
        throw 2;
    }
};
void f()
{
    C c;
    throw 1;
}
int main()
{
    try{f();}
    catch(int t)
    {
        cout << "catch " << t << endl;
    }
    return 0;
}
//程序会异常退出
```
另外如果在某个析构函数里抛出异常，而又没有在其内部捕获它，这个析构函数就不会完全运行。它会在异常抛出的地方停止。
所以应阻止异常传播到析构函数外面。
##构造函数与异常
c++只销毁构造完全的对象，所谓构造完全的对象是指它的构造函数被完全执行的对象。
所以如果在构造函数里抛出异常，并不会调用对应的析构函数。上面说过，异常机制会销毁所有的局部变量，但考虑这样一种情况：
```
class M
{
private:
    int *p1;
    int *p2;
public:
    M():p1(0),p2(0)
    {
        p1 = new int;
        p2 = new int;
    }
    ~M()
    {
        delete p1;
        delete p2;
    }
};
```
如果new p2的时候抛出了异常，p1的资源就泄漏了。
一种方法是在构造函数里使用try catch：
```
    M():p1(0),p2(0)
    {
        try
        {
            p1 = new int;
            p2 = new int;
        }
        catch(...)
        {
            delete p1;
            delete p2;
        }
    }
```
另一种更好的方法是将p1、p2所指的对象当做M所管理的资源来对待，通过别的对象来管理这些资源，比如auto_ptr。这样不仅免去了在析构函数中手动释放资源的必要，发生异常的时候构造函数还可以避免资源泄露。

[探索c++的new和delete](http://blog.csdn.net/liuyuan185442111/article/details/76937229)还探讨了operator new函数抛出异常的情形。
##参考
More Effective C++
[c++标准异常类别](http://www.cnblogs.com/zhuyf87/archive/2012/12/29/2839378.html)
[bad_exception](http://www.cplusplus.com/reference/exception/bad_exception/)
深度探索C++对象模型 第7章
[C++ 异常 与 ”为什么析构函数不能抛出异常“ 问题](http://www.cnblogs.com/zhyg6516/archive/2011/03/08/1977007.html)
[uncaught_exception](http://www.cplusplus.com/reference/exception/uncaught_exception/)