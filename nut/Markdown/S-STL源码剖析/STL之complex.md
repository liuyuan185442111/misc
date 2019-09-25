**note：本文相关源代码在std_complex.h中**

complex是一个类模板，实现了复数：

	template <class _Tp> class complex;
它有两个private成员变量，一个实部，一个虚部，它们的类型都是_Tp。
complex类中还包括或涉及这些部分：

- typedef _Tp value_type
- 构造函数
- real函数返回实部，imag函数返回虚部
- 重载了一些数学函数，如abs，log，sqrt等
- 重载了一些运算符，包括`=，+=，-=，*=， /=，+，-，*，/，==，!=，<<，>>`

complex类对float，double，long double做了specialization。将_Tp实例化为除float, double，long double之外的类型的行为是未定义的。
##Insertion operator的重载
```
template <typename _Tp, typename _CharT, class _Traits>
basic_ostream<_CharT, _Traits>&
operator<<(basic_ostream<_CharT, _Traits>& __os, const complex<_Tp>& __x)
{
    basic_ostringstream<_CharT, _Traits> __s;
    __s.flags(__os.flags());
    __s.imbue(__os.getloc());
    __s.precision(__os.precision());
    __s << '(' << __x.real() << "," << __x.imag() << ')';
    return __os << __s.str();
}
```
当我们这样写的时候：

	complex<float> t(1,2);
	cout << t;
就可以输出(1,2)了。

cout与这些语句有关：
```
//iostream文件
extern ostream cout;		/// Linked to standard output
//iosfwd文件
template<typename _CharT, typename _Traits = char_traits<_CharT> > class basic_ostream;
typedef basic_ostream<char> 		ostream;
//ostream文件
template<typename _CharT, typename _Traits>
class basic_ostream : virtual public basic_ios<_CharT, _Traits>…
```
cout就是`basic_ostream<char,char_traits<char> >`类型的对象。如此，operator<<函数的三个模板参数都可以推断出来了。
函数体部分，先通过basic_ostringstream这个模板类将输出放到一个缓冲区里，然后再用__os一起输出。
##Extraction operator的重载
和Insertion operator的重载类似：
```
template <typename _Tp, typename _CharT, class _Traits>
basic_istream<_CharT, _Traits>&
operator>>(basic_istream<_CharT, _Traits>& __is, complex<_Tp>& __x)
{
    _Tp __re_x, __im_x;
    _CharT __ch;
    __is >> __ch;
    if (__ch == '(')
    {
        __is >> __re_x >> __ch;
        if (__ch == ',')
        {
            __is >> __im_x >> __ch;
            if (__ch == ')')
                __x = complex<_Tp>(__re_x, __im_x);
            else
                __is.setstate(ios_base::failbit);
        }
        else if (__ch == ')')
            __x = complex<_Tp>(__re_x, _Tp(0));
        else
            __is.setstate(ios_base::failbit);
    }
    else
    {
        __is.putback(__ch);
        __is >> __re_x;
        __x = complex<_Tp>(__re_x, _Tp(0));
    }
    return __is;
}
```