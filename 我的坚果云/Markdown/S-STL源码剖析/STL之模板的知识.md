C++标准库是离不开模板的，STL占了C++标准库80%以上，下面有一些模板的知识。
##非类型的模板参数的限制
非类型的模板参数必须是常量表达式或一个具有外部链接属性的变量的地址（根据别人文章和编译器提示做出的推断）。比如：
```
template <int inst>
class MyClass {};
MyClass<5> x;
```
##Implicit Instantiate/隐式实例化
隐式实例化发生在函数模板的调用过程中，或使用类模板实例化对象的过程中。

在编译器可以推断出函数模板的模板参数时，模板参数可以省略。
编译器是无法推断出类模板的模板参数的，所以不能省略模板参数。
后面的显示实例化和具体化也是这样。
##Explicit Instantiate/显式实例化
要显式实例化函数模板，请在 template 关键字后接函数的声明（不是定义），且函数标识符后接模板参数：

	template float twice<float>(float original);
在编译器可以推断出模板参数时，模板参数可以省略：

	template float twice(float original);
要显式实例化类模板，请在 template 关键字后接类的声明（不是定义），且在类标识符后接模板参数：

	template class Array<char>;
显式实例化类时，所有的类成员也必须实例化。
##Specialize/特殊化/具体化
要特殊化函数模板，请在 template<> 关键字后接函数的定义，且在函数标识符后接模板参数：

	template<> void swap<job>(job &a, job &b) {……}
在编译器可以推断出模板参数时，模板参数可以省略：

	template<> void swap(job &a, job &b) {……}
要特殊化类模板，请在 template<> 关键字后接类的定义，且在类标识符后接模板参数：

	template<> class Array<char> {……};
##其他
不能对函数模板进行局部特化，因为他们认为可以通过函数重载来解决。

c++必知必会 item 48
可以只特化主模板成员函数的一个子集，这样在实例化时，类还是选用的主模板，但类的成员函数却可以选用特化版本。
但是，特化成员函数的接口必须和主模板提供的接口精确匹配。

c++必知必会 item 55 模板的模板参数
模板的参数不仅可以是普通类型、类，还可以是模板
stl中list的声明是这样的：

    template <typename _Tp, typename _Alloc = std::allocator<_Tp> >
    class list;
定义一个list得是这样：

    list <int, std::allocator<int> > m;
得手动协调元素和容器的类型，如果换成下面的方式：

    template <typename T, template <typename> class Alloc=std::allocator>
    struct List
    {
    };
Alloc是需要一个模板参数的模板类，定义简单了一些：

    List<int, std::allocator> n;

---

学习STL（c++ 98）的主要参考：
- [gcc 3.0源码](http://ftp.gnu.org/gnu/gcc/gcc-3.0/)
- 部分gcc 4.7.1源码
- [cplusplus](http://www.cplusplus.com/reference/)
- 《STL源码剖析》
- Internet，搜索引擎，他人博客

虽然本系列名为STL，但也涉及标准库的其他内容。
`<limits>`内容与c标准库中的`<limits.h>和<float.h>`相似，`<locale>`内容与c标准库中的`<locale.h>`相似，感觉用的也不多，先放一边。

IO库使用也不频繁，这里粗略说一下，与输入输出相关的头文件有：
文件名|说明
-|-
iosfwd|I/O功能的前导声明
iostream|标准iostream对象和操作
sstream|以串作为I/O对象的流
fstream|以文件作为I/O对象的流
ios|iostream基类
streambuf|流缓冲区
istream|输入流模板
ostream|输出流模板
iomanip|操控符
附一篇文章[《C++ 工程实践(7)：iostream 的用途与局限》](http://www.cppblog.com/Solstice/archive/2011/07/17/151224.html)作参考。