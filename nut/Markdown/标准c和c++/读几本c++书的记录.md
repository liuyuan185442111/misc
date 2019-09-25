##深度探索c++对象模型
①现在的编译器一般把vptr放在object的开始位置，把virtual base object放在object的末尾位置。
②constructor中对成员函数的调用一律转为静态调用（通过::的调用），无论是不是虚函数。
因为当派生类的constructor调用基类的constructor时，派生类对象还未构造完全，在基类的构造函数中如通过vptr调用派生类的函数，不恰当。
③class A;
class B:virtual public A...
class C:virtual public A...
class D:public B,public C...
实例化D时，只有D的constructor调用A的constructor，B和C的constructor不调用，可以通过这样的机制来实现：
创建对象时，如果有vbct（virtual base class table），则为构造函数添加一个参数，指示是否调用virtual base class的构造函数。
类似
D(D* const this, bool most_derived)
{
    if(most_derived) this->A::A();//通过vbct获得D的object中A的数据的起始位置
    this->B::B(false);
    this->C::C(false);//要对地址进行转化，获得D的object中C的数据起始位置
}
通过调用D(this, true);来构造对象即可实现只初始化一份A的对象。
④设置vptr的时机（from P216）:
在base class constructor调用操作之后，但是在程序员提供的代码或是member initialization list中所列的members初始化操作之前。
但如果构造函数中对成员函数的调用被转换为静态调用，那么vptr在什么时候设置也没什么大不了吧？
还是说构造函数中对虚成员函数的调用还是通过vptr，如果vptr已经被设置了的话？
⑤pure virtual function仅告诉编译器不能实例化该class，但其仍然可以有定义。
虽然不能通过生成实例来调用纯虚函数，但可以在派生类中静态调用。
如果析构函数被声明为纯虚，则必须有定义。
因为派生类在析构时，会依次调用基类的析构函数，所以抽象类必须提供析构函数的定义。
⑥指向virtual函数的指针，在实现上可能并不是地址，而是虚函数在虚表中的偏移。
⑦派生类的赋值操作符会调用基类和成员对象的赋值操作符，和默认构造函数、拷贝构造函数、析构函数一样。
⑧多继承、虚继承添加了太多的复杂度，无论是对编译器还是使用者，而且效率还很低，真应该取消掉。
⑨以上内容，有些臆测，仅供参考。

Template中的名称决议法(Name Resolution within a Template)
第7章的一小节，不好描述，也不好展现，就不说了。
##effective c++
item 04 确保对象在使用前得到初始化
不同编译单元的non-local static对象(就是定义在全局空间的static对象，不是定义在函数内部的local static对象)的初始化次序无法保证。
可以通过将它们转化为local static对象来解决，即把每个non-local static对象移入为它创建的专用函数中，函数要声明为static的，
这些函数返回一个它们所包含的对象的引用，于是程序员就可以调用这些函数，而不是直接使用那些对象。
那编译器为何不自动执行这个转化呢？一来这种转化要考虑多线程下的初始化不确定性，二来事情都让编译器做了，还要我们干什么。 ​​​​

item 11 确保当对象自我赋值时有良好的行为
技术包括比较两个对象的地址，临时存储原对象指针，copy-and-swap
先保存原对象指针，将原对象指针赋值，delete掉原对象指针，以防第二步中抛出异常
先制作一个副本，然后调用swap函数，制作副本时出问题也没什么影响

派生类中的函数会掩盖基类中的同名函数，无关参数，函数名的掩盖是语法分析时发生的，而function signature是编译时生效的。

private继承中的派生类不会像public继承那样可以向基类转换，因为public继承描述的是is-a关系，private继承不是。

item 47 traits技术
原书讲的很是透彻

item 46 通过friend实现参数的隐式转换
很自然的，一个有理数类的模板形式如下：
```
template <typename T>
struct Rational
{
    Rational(const T& numerator=0,const T& denominator=1)
    {
    }
}
template <typename T>
const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs)
{
	//do something
}
```
但是它不支持

	Rational<int> oneHalf(1,2);
	Rational<int> result = 2*oneHalf;
编译器无法从operator*调用动作中推测出使用哪个函数，因为在**template实参推导过程中从不将隐式类型转换函数纳入考虑**。解决这个问题只要利用一个事实：**template class内的friend声明式可以指涉某个特定函数**。Rational类变成了：
```
template <typename T>
struct Rational
{
    Rational(const T numerator=0,const T denominator=1)
    {
    }
    friend const Rational& operator*(const Rational& lhs, const Rational& rhs)
    {
	    //do something
    }
};
```
这项技术的一个有趣点是，我们虽然使用friend，却与friend的传统用途毫不相干。为了让类型转换可能发生于所有实参身上，我们需要一个non-member参数；为了令这个函数被自动具现化，我们需要将它声明在class内部；而在class内部声明non-member函数的唯一办法就是：令它成为一个friend。
这里必须把operator*定义也放到Rational内，因为如果只是一个声明，编译器在实例化Rational时，会寻找针对该实例的operator*的定义，然而找不到，链接就无法通过。如想把定义放到外面可以通过引入一个辅助函数，最后的形式如下：
```
//这个前置声明 work for doMultiply的声明
template <typename T>
struct Rational;

//不能在这里进行定义，因为我们还不知道Rational的结构
template <typename T>
const Rational<T> doMultiply(const Rational<T>& lhs, const Rational<T>& rhs);

template <typename T>
struct Rational
{
    Rational(const T numerator=0,const T denominator=1)
    {
    }
    friend const Rational& operator*(const Rational& lhs, const Rational& rhs)
    {
        return doMultiply(lhs, rhs);
    }
};

template <typename T>
const Rational<T> doMultiply(const Rational<T>& lhs, const Rational<T>& rhs)
{
    //do something
}
```
item 48 template元编程
在编译期计算阶乘的例子：
```
template <int n>
struct Factorial
{
    enum {value=n*Factorial<n-1>::value};
};

template <>
struct Factorial<0>
{
    enum {value=1};
};

int main()
{
    printf("%d\n",Factorial<10>::value);
}
```
但有一些局限： 
只能求在编译期就能确定的整数的阶乘； 
不能判别参数为负的情况； 
当嵌套层数过深时编译器会报 fatal error: template instantiation depth exceeds maximum of 900 (use -ftemplate-depth= to increase the maximum)

这个例子中并没有对模板进行具现，所以工作都放在了编译期。 
可以把enum换成static成员：
```
template <int n>
struct Factorial
{
    static int value;
};

template <int n>
int Factorial<n>::value=Factorial<n-1>::value*n;

template <>
struct Factorial<0>
{
    static int value;
};
int Factorial<0>::value=1;

int main()
{
    printf("%d\n",Factorial<3>::value);
}
```
##c++必知必会
c++必知必会27
dynamic_cast除了可以用来执行基类到派生类的转换，还可以用来对多重继承中不同基类进行转换

c++必知必会28
除了指向类的成员函数和成员变量的指针，一般来说指针的值就是对象的地址，但指向派生类的指针p0、指向基类1的指针p1、指向基类2的指针p2，其表示的地址可能不一样，p0==p1应为true，所以需要编译器对指针比较做一些额外的工作

c++必知必会32
```
#include <iostream>
using namespace std;

struct nocopy
{
    nocopy(int t) { cout << "param is " << t << endl; }
//private:
    nocopy(const nocopy &) { cout << "in copy constructor\n"; }
};

void fun(nocopy){}

int main()
{
    nocopy a(10);
    fun(a);
    fun(11);//直接调用默认构造函数
    fun(nocopy(12));//直接调用默认构造函数
    return 0;
}
/**输出是
param is 10
in copy constructor
param is 11
param is 12

如果不注释掉private，三个fun的调用都无法通过编译。
虽然第2、3个fun的调用，并没有用到复制构造函数，但编译器还是阻止了。
*/
```
##Effective STL
item 19
标准关联容器基于等价而不是相等。
对于两个对象x和y，如果按照关联容器c的排列顺序，每个都不在另一个的前面，那么称这两个对象按照c的排列顺序有等价的值。
使用默认比较函数的情况下，等价的意思是`!(x<y) && !(y<x)`。相等的意思是operator==(x,y)，虽然在数学上，这俩没差别，但等价是作为模板参数的predicate，相等是作为成员函数或友元函数的operator==。
关联容器的find成员函数使用等价，非成员的find方法使用相等。
http://www.sgi.com/tech/stl/StrictWeakOrdering.html

构造函数默认会调用成员对象的默认构造函数，但不会将内置类型（如int）进行初始化。
如果用户没有为对象定义默认的构造函数，编译器也不会对对象的成员进行初始化，基于此，如果用户没有显式地为内置类型成员进行初始化，编译器也不进行任何动作。

引用与指针相比，自带const属性。
从本质上来说，抽象数据类型的用途在于将编程语言扩展到一个特定的问题领域。
在类X的非常量成员函数中，this指针的类型为X *const。在类X的常量成员函数中，this指针的类型为const X *const。

指向非静态类成员的指针指向的是一个类的特定成员，而不是指向一个特定对象里的特定成员。
当对指向非静态成员函数的指针解引用，需要将对象的地址（或用作计算）用作this指针的值，进行函数调用，以及用作其他用途。
    void (X::*p)(int) = &X::f;
    X x;
    (x.*p)(5);
虚拟性是成员函数自身的属性，而不是指向它的指针所具有的属性。一个指向成员函数的指针必须存储一些信息，诸如它所指向的成员函数是不是虚拟的，到哪里去寻找适当的虚函数表。

item 6 c++尽可能地解释为函数声明
看这样一个简单的例子：
```
class Widget
{
public:
    Widget() {}
    void work()
    {
        std::cout << "work\n";
    }
};

int main()
{
    Widget w();//此处解析为函数声明
    w.work();
    return 0;
}

error: request for member 'work' in 'w', which is of non-class type 'Widget()'
```
错误提示里的 Widget() 是返回值是Widget参数为空的函数。
书里的例子：
```
#include <iostream>
#include <iterator>
#include <list>
#include <vector>
using namespace std;

int main()
{
    istream_iterator<int> databegin(cin);
    istream_iterator<int> dataend;
    list<int> data(istream_iterator<int>(cin),istream_iterator<int>());
    cout << "sizeof data is " << data.size() << endl;
    for(list<int>::iterator it=data.begin(); it!=data.end(); ++it)
    {
        cout << *it << ' ';
    }
    return 0;
}
error: request for member 'size' in 'data', which is of non-class type
'std::list<int>(std::istream_iterator<int>, std::istream_iterator<int> (*)())'
```
错误提示里看出，`istream_iterator<int>(cin)`被解析为参数名为cin的`istream_iterator<int>，istream_iterator<int>()`被解析为省略参数的函数指针。
把形式参数的声明用括号括起来是非法的，给函数参数加上括号却是合法的，所以给任意一个参数套上括号就可以了： 
`list<int> data((istream_iterator<int>(cin)),istream_iterator<int>());`

但是出现了新问题，输入的第一个整数被吃掉了！ 看了[我以前对迭代器的记录](http://blog.csdn.net/liuyuan185442111/article/details/45848299)：
```
	istream_iterator(istream_type& s) : _M_stream(&s)
    {
        _M_read();
    }
    reference operator*() const
    {
        return _M_value;
    }
    istream_iterator& operator++()
    {
        _M_read();
        return *this;
    }
```
定义databegin的时候读取了一次，定义data的时候临时变量`istream_iterator<int>(cin)`又读取了一次，对其进行解引用的时候，返回的是第二次读取的值！
##More Effective C++
条款24
在效率上虚函数的调用只比非虚函数的调用差一点点。虚函数意味着运行时刻决定调用哪个函数，内联意味着在编译时刻用被调用函数的函数体来代替被调用函数，所以，虚函数只能放弃内联。（通过对象调用虚函数时可以被内联，但大多虚函数是通过指针或引用调用）
一个类型有一个对应的type_info对象，typeid对指针或引用进行决断，先判断其指向的对象有没有虚指针，如果没有则可在编译期就进行决断，如果有虚指针，则可能是通过vptr找到虚函数表，虚函数表的某个位置可能存储了指向type_info对象的指针。
另外编译器可能还额外维护了一个映射关系，存储了所有类型和其type_info对象的对应关系。
条款26
1.只有在public继承中，才能让基类指针或引用指向派生类。
2.访问权限和继承方式的组合
private + public继承 => 派生类无权访问
private + protected继承 => 派生类无权访问
private + private继承 = > 派生类无权访问

protected + private继承 = > private
public + private继承 = > private

protected + protected继承 => protected
public + protected继承 => protected

protected + public继承 => protected
public + public继承 => public

在后6种情形中，派生类可以在public访问控制中使用“using 基类::基类成员”，使这些基类成员在派生类中变为public的。
（参考http://www.jb51.net/article/41642.htm）