##RTTI
c++的RTTI（Run-Time Type Information）的相关内容包括typeid、dynamic_cast这两个关键字，和头文件&lt;typeinfo>。

type_info在&lt;typeinfo>中定义，必须在typeid运算符的使用之前包含此头文件。

type_info的public成员有：
```cpp
virtual ~type_info();
/** 返回一个实现定义的字符串, 不可移植, 同种类型的type_info对象
的name()返回的字符串相同, 不同类型对应的字符串则不同 */
const char* name() const;
/** 是否before参数标识的类型, 不同编译器实现可能不同,
不同程序之间返回值可能也不同, 同一程序不同时间运行结果可能也不一样 */
bool before(const type_info& arg) const;
bool operator==(const type_info& arg) const;
bool operator!=(const type_info& arg) const
```
type_info的默认构造函数，拷贝构造函数，赋值运算符都是private或protected的。这样，你就不能建立一个type_info类型的对象，只能通过typeid运算符来获得type_info对象。

typeid的作用是在程序运行时，返回对象的类型，主要用来在多态中判断一个基类指针或引用是否指向派生类对象，用起来像一个函数，返回值类型是const type_info &。同sizeof类似，typeid可接受一个类型或变量。

typeid(t)中，当t是对象时，只有对象t是引用且t有虚指针时才会通过虚指针去寻找t的type_info，其他情况下，直接在编译时期确定类型。 typeid(*p)中，当p是指向对象的指针时，也是类似。

可以用typeid来简陋地模拟实现dynamic_cast的部分功能：
```
template <typename T, typename U>
struct my_dynamic_cast_t
{
    T* operator()(U*p)
    {
        return 0;
    }
};

template <typename T, typename U>
struct my_dynamic_cast_t<T*, U>
{
    T* operator()(U*p)
    {
        if(typeid(*p) == typeid(T))
            return static_cast<T*>(p);
        else return NULL;
    }
};

template <typename T, typename U>
T my_dynamic_cast(U* p)
{
    return my_dynamic_cast_t<T,U>()(p);
}
```
&lt;typeinfo>中还定义了两个异常：
```cpp
// 强制转换为引用类型失败, dynamic_cast运算符引发bad_cast异常
class bad_cast : public exception
{
  public:
    bad_cast() throw() { }
    virtual ~bad_cast() throw();
    virtual const char* what() const throw();
};

// 如果交给typeid的参数是空指针, 抛出此异常
class bad_typeid : public exception
{
  public:
    bad_typeid () throw() { }
    virtual ~bad_typeid() throw();
    virtual const char* what() const throw();
};
```
抛出bad_typeid的情形如下，注意必须有虚指针才会抛出异常，否则如上所说，在编译期就能决断出*p的类型来：
```
struct b
{
    virtual ~b(){}
};

int main()
{
    b *p = NULL;
    typeid(*p);
    return 0;
}
```
##类型转换操作符
c++里有4个强制类型转换操作符，分别是static_cast, dynamic_cast，const_cast，reinterpret_cast。
###reinterpret_cast
reinterpret是重新解释的意思，reinterpret_cast仅用来对内存地址进行变换操作，reinterpret_cast会产生一个新的值，这个值会有与原始参数有完全相同的比特位。

reinterpret_cast可用作：
任意指针类型到一个足够大的整数类型的转换
整数类型到任意指针类型的转换
任意指针类型之间的互相转换
任意类型左值到任意类型引用的转换，如：

    vector<int> v;
    reinterpret_cast<int&>(v);//虽然能通过编译，但没什么意义
###const_cast
`const_cast<type-id>(expression) `
该运算符用来修改类型的const或volatile属性。
type-id必须是指针，引用，或指向数据成员的指针。
###static_cast
没有运行时类型检查来保证转换的安全性，用途有：
①	用于类层次结构中基类和派生类之间指针或引用的转换。
	进行上行转换（把派生类的指针或引用转换成基类表示）是安全的；
	进行下行转换（把基类指针或引用转换成派生类表示）时，由于没有动态类型检查，所以是不安全的。
	一般来说下行转换应使用dynamic_cast，但如果能保证转换是安全的，则可以使用static_cast，而且效率还高一些。
②用于基本数据类型之间的转换，如把int转换成char，把int转换成enum，把double转换成int，这种转换的安全性也要开发人员来保证。
③把空指针转换成目标类型的指针。
④把任何类型的表达式转换成void类型。
⑤如果一个类拥有只有一个参数的构造函数或类型转换运算符，则可以用static_cast来调用二者进行转换。
###dynamic_cast
`dynamic_cast<type-id>(expression)`
以上三个操作符在编译时进行转换，dynamic_cast则在运行时进行类型转换，该转换符主要用于将一个指向派生类的基类指针或引用转换为派生类的指针或引用。

dynamic_cast只能用于指针或引用，type-id必须是pointer or reference to class。dynamic_cast支持派生类指针/引用到基类指针/引用的转换，但会在编译时进行，`c *p;dynamic_cast<c*>(p);`这种无聊操作也能通过编译。除了这两种情况，expression的类必须是多态的，即必须有虚函数。对type-id的类却没有要求，可以和expression完全不相关。
用dynamic_cast对没有关系的两个类指针或引用进行转换，当然会转换失败，但能编译通过，static_cast则是直接编译失败。

dynamic_cast还可用来对多重继承中不同基类指针/引用进行转换（cross cast）（c++必知必会27 ）。

特别的，把一个指针dynamic_cast成void*类型，转换后的指针指向原来指针所指向对象的内存开始处。一个用处是来判断两个指针是不是从同一个类派生而来：
```
template<typename X, typename Y>
bool operator==(const X* px, const Y* py) {
     return dynamic_cast<void*>(px) == dynamic_cast<void*>(py);
}
```
More Effective C++ 条款27说明了另一种用法，实现了一个HeapTracked类，判断一个从该类派生而来的类是否分配在堆上。

如果dynamic_cast转换失败，如果是指针则反回一个0值，如果是转换的是引用，则抛出一个bad_cast异常。
如果对引用进行转换时不想抛出异常，可以用typeid和static_cast来代替。（深度探索C++对象模型第7章）
##reinterpret_cast和static_cast的不同
```cpp
class A
{
public:
    int m_a;
};

class B
{
public:
    int m_b;
};

class C : public A, public B {};

int main()
{
    C c;
    printf("%p, %p, %p", &c, reinterpret_cast<B*>(&c), static_cast<B*>(&c));
    return 0;
}

输出：
0028FF18, 0028FF18, 0028FF1C
```
reinterpret_cast就是对地址强制重新解释而已，static_cast则进行了安全的转换。
##参考
[C++类型转换方式总结](http://www.cnblogs.com/ider/archive/2011/08/05/cpp_cast_operator_part6.html)
c++必知必会
More Effective C++ 条款5
深度探索C++对象模型 第7章