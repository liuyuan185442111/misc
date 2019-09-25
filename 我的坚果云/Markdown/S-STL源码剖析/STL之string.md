先看string的定义（stringfwd.h）：
```
//char_traits的声明
template <class _CharT>
struct char_traits;

//char_traits的specialization的声明
template<> class char_traits<char>;
template<> class char_traits<wchar_t>;

//allocator的声明
template <typename _Alloc>
class allocator;

//basic_string的声明
template <typename _CharT, typename _Traits = char_traits<_CharT>,
         typename _Alloc = allocator<_CharT> >
class basic_string;

//basic_string的typedef
typedef basic_string<char>    string;
typedef basic_string<wchar_t> wstring;
```
string是basic_string的一个实例。此外还需要char_traits，allocator模板类的支持。allocator的说明见于[《STL之空间配置器》](http://blog.csdn.net/liuyuan185442111/article/details/45743345)。
##char_traits（char_traits.h）
char_traits是一个模板类，它列出了字符的属性，为字符和字符序列的某些操作提供了定义。其实就是将一些字符的常用操作，如赋值、比较、查找、移动、复制等，封装为函数，以配合STL。
char_traits是一个struct，它有5个类型定义和若干static函数，它有两个specialization，分别是对char和wchar，char_traits被用于basic_string和input/output stream。

基本上了解`template <> struct char_traits<char>;`就可以了，因为对wchar的specialization不过是将一些操作由char换成wchar。char_traits将一些操作进行了保守估计，反过来也就是说针对char的specialization进行了一些特别的优化。
```
template <class _CharT> struct char_traits;
template <> struct char_traits<char>;
template <> struct char_traits<wchar_t>;
```
###member types
|类型|说明|
|-|-|
|char_type|模板参数_CharT|
|int_type|包含所有_CharT类型字符的整型，包括eof()|
|off_type|A type that behaves like streamoff|
|pos_type|A type that behaves like streampos|
|state_type|Multibyte transformation state type, such as mbstate_t|
###member functions
|函数|说明|
|-|-|
|eq|等于操作|
|lt|小于操作|
|length|获取字符序列的长度|
|assign|字符赋值|
|compare|字符序列比较|
|find|查找字符第一次出现的位置|
|move|移动|
|copy|复制|
|to_char_type|int_type到char_type|
|to_int_type|char_type到int_type|
|eq_int_type|int_type的等于操作|
|eof|返回End-of-File字符|
|not_eof|判断是不是End-of-File字符，如果是返回0，如果不是返回本身|
###部分源码
下面附上`template <> struct char_traits<char>;`的源码：
```
template<>
struct char_traits<char>
{
    typedef char 		char_type;
    typedef int 	    int_type;
    typedef streampos 	pos_type;
    typedef streamoff 	off_type;
    typedef mbstate_t 	state_type;

    static void assign(char_type& c1, const char_type& c2)
    {
        c1 = c2;
    }

    static bool eq(const char_type& c1, const char_type& c2)
    {
        return c1 == c2;
    }

    static bool lt(const char_type& c1, const char_type& c2)
    {
        return c1 < c2;
    }

    static int compare(const char_type* s1, const char_type* s2, size_t n)
    {
        return memcmp(s1, s2, n);
    }

    static size_t length(const char_type* s)
    {
        return strlen(s);
    }

    static const char_type *find(const char_type* s, size_t n, const char_type& a)
    {
        return static_cast<const char_type*>(memchr(s, a, n));
    }

    static char_type *move(char_type* s1, const char_type* s2, size_t n)
    {
        return static_cast<char_type*>(memmove(s1, s2, n));
    }

    static char_type *copy(char_type* s1, const char_type* s2, size_t n)
    {
        return static_cast<char_type*>(memcpy(s1, s2, n));
    }

    static char_type *assign(char_type* s, size_t n, char_type a)
    {
        return static_cast<char_type*>(memset(s, a, n));
    }

    static char_type to_char_type(const int_type& c)
    {
        return static_cast<char_type>(c);
    }

    static int_type to_int_type(const char_type& c)
    {
        return static_cast<int_type>(static_cast<unsigned char>(c));
    }

    static bool eq_int_type(const int_type& c1, const int_type& c2)
    {
        return c1 == c2;
    }

    static int_type eof()
    {
        return static_cast<int_type>(EOF);
    }

    static int_type not_eof(const int_type& c)
    {
        return (c == eof()) ? 0 : c;
    }
};
```
##basic_string
我发现basic_string的实现挺复杂，代码在basic_string.h和basic_string.tcc中。这里就只介绍下basic_string的接口。
###成员类型
```
public:
typedef _Traits								traits_type;
typedef typename _Traits::char_type			value_type;
typedef _Alloc								allocator_type;
typedef typename _Alloc::size_type			size_type;
typedef typename _Alloc::difference_type	difference_type;
typedef typename _Alloc::reference			reference;
typedef typename _Alloc::const_reference	const_reference;
typedef typename _Alloc::pointer			pointer;
typedef typename _Alloc::const_pointer		const_pointer;
typedef __normal_iterator<pointer, basic_string>		iterator;
typedef __normal_iterator<const_pointer, basic_string>	const_iterator;
typedef reverse_iterator<const_iterator>	const_reverse_iterator;
typedef reverse_iterator<iterator>			reverse_iterator;
```
###静态常量
	public:
	static const size_type npos = static_cast<size_type>(-1);
npos是allocator可以分配的最大字节数。
当作为basic_string成员函数的长度参数时，表示“直到字符串的结尾”。
当作为一个返回值时，常表示没有匹配。
###成员函数和相关的非成员函数
见这个页面：[basic_string](http://www.cplusplus.com/reference/string/basic_string/)。