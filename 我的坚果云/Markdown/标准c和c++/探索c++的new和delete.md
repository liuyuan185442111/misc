c++的new是语言自定义的操作符，这个操作符的行为包含两件事，而且用户不能改变：
第一件事，调用operator new()分配内存。所以通常说的重载new其实重载的是operator new()这个函数，我们无法重载new操作符。
第二件事，调用要new的对象的所属类的构造函数初始化第一步中分配的内存。
delete操作符的行为也是类似，它先调用析构函数，然后调用operator delete()释放内存。

**操作符new和delete能且只能这样调用**：
new(可能的参数或空) 内置数据类型或类;
delete 一个指针;

操作符delete之后的指针会作为参数传递给operator delete()，操作符new如果存在参数，也会传递给operator new()。

new[]、delete[]和new、delete是类似的，所以这里只说明new和delete。
下面是头文件`<new>`里operator new()和operator delete()的所有声明（除去一些旁枝末节），这些都是全局的，不包含在namespace std里，而且默认编译器会自动包含头文件`<new>`，并不需要显式include：
```
void* operator new(std::size_t) throw(std::bad_alloc);
void operator delete(void*) throw();
void* operator new(std::size_t, const std::nothrow_t&) throw();
void operator delete(void*, const std::nothrow_t&) throw();
inline void* operator new(std::size_t, void *p) throw() {return p;}
inline void operator delete(void*, void*) throw() { }
```
最后两个称为placement版，是从p所指向的内存区域分配内存，p可以是一个char数组、int数组之类的。它们是内联的，由于内联函数只能有一个定义，所以无法在全局里重载它们。而且，由于无法给delete操作符传递第二个参数（delete操作符只有一种调用方式），所以placement operator delete()是无法通过delete操作符调用的，这意味着，我们只能显式的调用析构函数和placement operator delete()，而placement operator delete()也是空的，给它传递什么参数都无所谓。这两个函数并没有分配内存的操作，所以它们不会throw异常。

operator new()的第一个参数是编译器要分配内存的大小，是由编译器计算并且隐式传递的，并不需要我们插手。在编译器的内部实现中，传入new的尺寸值可能是所需内存的大小s加上一个delta，这个delta是编译器的内部实现定义的某种额外开销。为什么delete操作符不需要第二个尺寸参数呢？因为系统“记住”了分配内存的大小。

如果new操作符不能分配出内存，默认情况下会抛出一个bad_alloc异常对象，然而如果我们自定义了内存耗尽时的处理方法(new_handler)，则会执行这个方法。定义完处理方法后需要用set_new_handler()来登记，set_new_handler定义在namespace std里，它返回旧的handler：`new_handler set_new_handler(new_handler) throw();`。

还有一个不会抛出异常的operator new()，namespace std里定义了nothrow_t和一个nothrow_t类型的变量供它使用：

	struct nothrow_t { };
	extern const nothrow_t nothrow;

##operator new( )和operator delete( )的重载
我们可以将这两个函数重载为全局的，也可以**重载为类的成员函数**。
如果非要把它们重载为全局的，最好不要覆盖默认的operator new() 和operator delete()，可以多加一个参数来避免覆盖默认的版本，例如：
```
void *operator new(size_t n, int m)
{
	return ::operator new(n);
}
```
是的，正如你看到一样，重载后的版本可以带多个参数。
如果覆盖了默认的operator new()，不要在里面调用::operator new()，而应调用malloc()，否则就会发生递归调用。
如果将它们重载为类的成员函数，它们默认成为static的，这很明显，调用它们的时候构造函数都没有调用，this指针还不存在，但我们不妨手动给它加上static关键字，其他方面，比如继承，则和成员函数一样。。

new操作符后面可以有一个括号，可以带很多参数，这些参数都会传递给operator new()，但operator new()的第一个参数必须是size_t类型。实际上，placement operator new()也不过是operator new()的一个重载版本。

operator delete()也可以进行重载，不过我们无法通过delete操作符来调用有多个参数的operator delete()，因为delete操作符的调用只能接受一个指针类型的参数，使用默认的new没问题，但是当对operator new()进行重载时，我们没有优美的方式来调用对应的operator delete()。Stroustrup的回答在此[Is there a “placement delete”?](http://www.stroustrup.com/bs_faq2.html#placement-delete) （广义的placement new指的是operator new的重载版本，狭义的是头文件中定义好的`inline void* operator new(std::size_t, void* p) throw();`）。

但是对应的operator delete()有时却是必须的。当使用new操作符时，会发生两件事情，一是调用operator new()获取一些内存，二是调用对象的构造函数，如果在第二步时构造函数抛出异常，编译器必须负责回收掉operator new()获取的那些内存，编译器会寻找一个与operator new()的参数标对应的operator delete()来调用，如果找不到则什么也不做。operator delete()的第一个参数是void*类型，operator new()的第一个参数是size_t类型，二者对应是说它们的第一个参数以后的参数数目和类型相同。所以，如果没有定义对应的operator delete()，极有可能会发生内存泄漏。比如：
```
class ND
{
public:
    static void *operator new(size_t size, int)
    {
        cout << "in ND::operator new\n";
        return ::operator new(size);
    }
    static void operator delete(void *p, int)
    {
        cout << "in ND::operator delete\n";
        ::operator delete(p);
    }
    ND()
    {
        cout << "in ND::ND construtor\n";
        throw(9);
    }
    ~ND()
    {
        cout << "in ND::ND destrutor\n";
    }
};

int main(int argc, char **argv)
{
    try
    {
        ND *p = new(2) ND;
        p->~ND();
        ND::operator delete(p,3);
    }
    catch(...)
    {
        return -1;
    }
    return 0;
}

输出是
in ND::operator new
in ND::ND construtor
in ND::operator delete
```
但如果上面的代码中没有定义`static void operator delete(void *p, int);`，则会造成内存泄漏。
在上面的代码中，我还显示的调用了ND的析构函数和ND::operator delete()，以完成原本delete操作符应完成的事。

所以系统头文件`<new>`中的operator new()和operator delete()都是成对的。当然，如果构造函数中确实不会抛出异常，不定义与operator new()对应的operator delete()也没问题，比如：
```
class ND
{
public:
    static void *operator new(size_t size, int)
    {
        cout << "in ND::operator new\n";
        return ::operator new(size);
    }
    /*static void operator delete(void *p, int)
    {
        cout << "in ND::operator delete\n";
        ::operator delete(p);
    }*/
    ND() throw()
    {
        cout << "in ND::ND construtor\n";
    }
    ~ND()
    {
        cout << "in ND::ND destrutor\n";
    }
};

int main(int argc, char **argv)
{
    ND *p = new(1) ND;
    delete p;
    return 0;
}

输出是
in ND::operator new
in ND::ND construtor
in ND::ND destrutor
```
注意这里必须把operator delete()注释掉，因为如果在class内声明任何operator new()，它就会掩盖全局空间里的那些operator new()，operator delete()也是如此。
##virtual destructor和operator delete
《c++必知必会》item 36提供了一个例子：
```#include <iostream>
using namespace std;

struct base
{
    //virtual
    ~base(){}
    void operator delete(void *p)
    {
        cout << "in base delete\n";
        ::operator delete(p);
    }
};
struct derive : public base
{
    //delete 1
    void operator delete(void *p)
    {
        cout << "in derive delete\n";
        ::operator delete(p);
    }
    //delete 2
    void operator delete(void *p, size_t s)
    {
        cout << "size is " << s << ", in derive delete\n";
        ::operator delete(p);
    }
};

/**
输出为in base delete,然而如果base的析构被声明为virtual,输出变为in derive delete
注意,仅限于将析构函数声明为virtual才行!如果只是将普通成员函数声明为virtual,没有用!

如果将delete1删掉,则会调用delete2,二者是一样的,不过delete1优先级高一些.
delete2的第二个参数用于保存正被删除的对象大小,这种信息在实现自定义内存管理时往往很有用.
*/

int main()
{
    base *p = new derive;
    delete p;
    return 0;
}
```
《深度探索C++对象模型》6.2节也对此进行了讨论：
new B[2];
开辟了一块内存，大小为2*sizeof(B)，并对每个B都执行了构造函数，然后返回该内存块的起始地址。
delete p;
先对p指向的内存调用p指向类型的析构函数（如果该析构是virtual的，则会通过vptr调用派生类的析构函数，这是编译器在编译时扩充的操作），再free掉该内存块。
delete [] p;
对于new []，（可能）编译器会维护一个map,放置指针及数组元素的个数；执行delete [] p;时，会找到p对应的数组元素的个数，然后对p指向的内存块，（以sizeof(*p)为跃度）依次调用p指向类型的析构函数。
##动态分配一个二维数组
这里提供了一种方式：
```
int x = 3, y = 4;
int **p = new int*[x];//创建一个动态 int* 型数组
for (int i = 0; i < x; ++i)
    p[i] = new int[y]; //再创建一个动态 int 型数组

for (int i = 0; i < x; ++i)
{
    delete [] p[i];//由里至外，进行释放内存。
    p[i] = NULL;//不要忘记，释放空间后p[i]不会自动指向NULL值，还将守在原处，只是释放内存而已，仅此而已
}
delete [] p;
p = NULL;
```
如果要使二维数组占用一块连续内存，可以这样：
```
int *array1 = new int[x*y];
int **array2;
array2 = new int* [x];
for(int i=0; i<x; ++i)
    array2[i] = array1 + i*y;

array2[1][2] = 4;

delete[] array1;
delete[] array2;
```
##部分源代码
```
//<new>
namespace std 
{
  class bad_alloc : public exception 
  {
  public:
    bad_alloc() throw() { }
    virtual ~bad_alloc() throw();
    virtual const char* what() const throw();
  };

  struct nothrow_t { };
  extern const nothrow_t nothrow;

  /// If you write your own error handler, it must be of this type.
  typedef void (*new_handler)();

  /// Takes a replacement handler as the argument, returns the
  /// previous handler.
  new_handler set_new_handler(new_handler) throw();
} // namespace std

void* operator new(std::size_t) throw(std::bad_alloc);
void* operator new[](std::size_t) throw(std::bad_alloc);
void operator delete(void*) throw();
void operator delete[](void*) throw();
void* operator new(std::size_t, const std::nothrow_t&) throw();
void* operator new[](std::size_t, const std::nothrow_t&) throw();
void operator delete(void*, const std::nothrow_t&) throw();
void operator delete[](void*, const std::nothrow_t&) throw();

// Default placement versions of operator new.
inline void* operator new(std::size_t, void* __p) throw()
{ return __p; }
inline void* operator new[](std::size_t, void* __p) throw()
{ return __p; }

// Default placement versions of operator delete.
inline void operator delete  (void*, void*) throw() { }
inline void operator delete[](void*, void*) throw() { }
```
```
// new的主要工作就是调用malloc
void* operator new(size_t sz) throw(std::bad_alloc)
{
    void *p;
    /* malloc(0) is unpredictable; avoid it.  */
    if(sz == 0)
        sz = 1;
    p = (void *)malloc(sz);
    while(p == 0)
    {
        new_handler handler = __new_handler;
        if(!handler)
            throw bad_alloc();
        handler();
        p = (void *)malloc(sz);
    }

    return p;
}

// new[]的内容就是调用new
void* operator new[] (size_t sz) throw(std::bad_alloc)
{
    return ::operator new(sz);
}

// delete的内容是调用free
void operator delete (void *ptr) throw()
{
    if(ptr) free(ptr);
}

// delete[]的内容和delete相同
void operator delete[] (void *ptr) throw()
{
    if(ptr) free(ptr);
}
```
##参考
《c++必知必会》
《深度探索C++对象模型》
c++的new和delete
http://blog.csdn.net/liuyuan185442111/article/details/43027867
C与C++中的异常处理(new/delete)
http://blog.csdn.net/guoxiaoqian8028/article/details/8211775