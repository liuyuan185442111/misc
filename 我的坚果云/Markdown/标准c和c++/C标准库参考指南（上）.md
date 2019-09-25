##前述
c标准库参考指南
A guide to the stranded C library

本指南主要基于《C标准库》（P.J.Plauger，2009-7-1，人民邮电出版社），在用到c标准库的时候可以参考，同时本指南也有部分实现细节。由于时间和精力有限，很多方面被忽略掉了，比如硬件相关部分、区域化设置、大部分数学函数、内存分配算法、宽字符和多字符部分、其他复杂的算法等。

东方肖遥
185442111@qq.com
http://weibo.com/u/1901302255
http://blog.csdn.net/liuyuan185442111
2014年9-10月 于北邮

v1.1，稍微做了一些修改，2015年2月，于家
v1.2，稍作修改，2015年6月，于学校
v1.3，稍加整理格式，以作备份，2017年9月，于公司

最初c89标准库只有15个头文件：
头文件|说明
-|-
assert.h|断言相关
ctype.h|字符类型判断
errno.h|标准错误机制
float.h|浮点限制
limits.h|整型限制
locale.h|本地化接口
math.h|数学函数
setjmp.h|非本地跳转
signal.h|信号相关
stdarg.h|可变参数处理
stddef.h|一些宏和类型的定义
stdio.h|标准I/O库
stdlib.h|标准工具库
string.h|字符串及内存处理
time.h|时间相关
后来标准库扩充了一些，但这15个无疑是最常用的，《C标准库》一书也仅介绍了这15个头文件，在此我也仅介绍这15个头文件。 
##断言（assert.h）
此头文件只包含一个assert宏，从用法上看它十分类似一个返回值是void的函数。如果条件为假，assert将输出一些信息并调用abort函数退出程序。
一个可能的实现：

	#ifdef NDEBUG
	#define assert(x) ((void)0)
	#else
	void _assert(const char*, const char*, int);
	#define assert(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
	#endif
assert应该只用在debug版本中。如果`#define NDEBUG`，之后的assert将会失效；可以重新`#undef NDEBUG`，但需要再次`#include assert.h`才能使后面的assert生效。
因为需要被多次include，所以本头文件不需要通过“#ifndef机制”防止被多次包含。
##字符类型判断（ctype.h）
此头文件定义了一批c语言字符分类函数。
以下假定使用的字符集是ASCII。
这些函数都只有一个参数，且参数和返回值均为int类型，比如`int isalpha(int);`。
为了提高速度，很多函数的实现可能会采用查表法，在这里只描述其功能，而不细述其实现。
下表列出了这些函数：
![函数说明](http://img.blog.csdn.net/20150622161220334)
##标准错误机制（errno.h）
此头文件定义了一个表示错误信息的errno，errno等价于一个全局整型变量。
另外还定义了表示错误编码的一些宏，他们都以"E"开头。比如：
![错误编码](http://img.blog.csdn.net/20150622161325594)
##浮点限制（float.h）
此头文件定义了浮点型变量的一些极限值和设置。
##整型限制（limits.h）
此头文件定义了整型变量的一些极限值和设置。
##本地化（locale.h）
c语言支持本地化设定，如本地格式时间和货币符号。
此头文件包含一个类型定义，两个函数声明和一些宏定义。
###类型定义
	struct lconv;
其成员中有三个会影响浮点数的表达形式：

	char *decimal_point; // 格式化非货币量中的小数点字符
	char *thousands_sep; // 用来对格式化的非货币量中小数点前面的数字进行分组的字符
	char *grouping; // 用来说明格式化的非货币量中每一组数字的数目
其他成员影响货币的表达形式。
###函数声明
	struct lconv *localeconv(void);
标准库定义了一个struct lconv类型的全局变量，此函数返回指向此变量的指针。

	char *setlocale(int category, const char *locale);
此函数用来设定或恢复本地化配置。
其中category参数可以为LC_COLLATE,LC_CTYPE,LC_MONETARY,LC_NUMERIC,LC_TIME,LC_ALL。
参数|说明
-|-
LC_ALL|设置所有信息
LC_COLLATE|影响strcoll和strxfrm函数
LC_CTYPE|影响所有字符函数
LC_MONETARY|影响由localeconv函数提供的货币信息
LC_NUMERIC|影响十进制小数格式和localeconv函数提供的信息
LC_TIME|影响strftime函数
如果locale是一个指向字符串的指针且选择有效，则返回一个和新的区域设置指定的category相联系的字符串的指针；如果选择因故无法实现，则返回一个空指针且程序的区域设置不改变。如果locale参数是一个空指针，则返回一个和当前区域设置的category相联系的字符串的指针，程序的区域设置不改变。
`"C"`为编译器指定的最小环境值，`""`指定了实现定义的本地环境，在程序启动时，程序执行与“`setlocale(LC_ALL, "C");`”等效的代码。

此外，本文件还定义了NULL宏。gcc是这样说的：According to C89 std, NULL is defined in locale.h too。

经测试，decimal_point的值会影响gcc中printf输出中的小数点，vc下无影响，thousands_sep和grouping不会对printf产生影响。
##数学函数（math.h）
此头文件里的函数都是由专业人士编写，普通用户只需要了解怎么用就可以了。
此头文件定义了一个宏HUGE_VAL，它展开为一个正的double类型的表达式。
还声明了一些数学函数，除了frexp、ldexp、modf，其他函数的参数和返回值都是double类型。

数学函数常会发生定义域错误和值域错误：
如发生定义域错误，errno设为EDOM。
如发生值域错误：如果结果上溢，函数返回HUGE_VAL的值，符号与函数的正确值相同（除了tan函数），errno设为ERANGE；如果结果下溢，函数返回0，errno是否取宏ERANGE的值取决于实现。

大部分函数的用法显而易见，所以仅把这些函数的声明列出来，对另外一些函数则进行详细说明。
###三角函数和双曲函数
	double sin(double x);
	double cos(double x);
	double tan(double x);
	double asin(double x);
	double acos(double x);
	double atan(double x);
	double atan2(double y, double x);
	double sinh(double x);
	double cosh(double x);
	double tanh(double x);
###指数函数、对数函数、幂函数
	double exp(double x); // e的x方
	double log(double x); // 自然对数
	double log10(double x); // 以10为底x的对数
	double pow(double x, double y); //x的y次方
	double sqrt(double x); // x的平方根

	double frexp(double value, int *exp);
此函数把一个浮点数分为一个规格化小数和一个2的整数幂。假定返回值为x，x的范围为[1/2,1)，或者为0。参数和返回值满足value=x*2^exp，如果value为0，x和*exp都为0。

	double ldexp(double x, int exp);
此函数计算一个浮点数和2的整数幂的乘积，它返回x*2^exp的值。

	double modf(double value, double *iptr);
此函数把value分为整数和小数部分，他们的符号和value的符号相同，小数部分作为返回值，整数部分存放在iptr指向的double类型的对象中。
###取整函数、绝对值函数、取余函数
	double ceil(double x); // 不小于x的最小整数，ceil天花板。
	double floor(double x); // 不大于x的最大整数，floor地板。
	double fabs(double x); // x的绝对值
	double fmod(double x, double y); // x除以y的浮点余数
##非本地跳转（setjmp.h）
此头文件包含：

	typedef int jmp_buf[16];
	int setjmp(jmp_buf);
	void longjmp(jmp_buf, int);
setjmp和longjmp可能实现为宏定义。
非本地跳转的原理非常简单：
1.setjmp(j)设置“jump”点，用正确的程序上下文填充jmp_buf对象j。这个上下文包括程序存放位置，栈和框架指针，其它重要的寄存器和内存数据。当初始化完jump的上下文，setjmp()返回0值。
2.以后调用longjmp(j,r)的效果就是一个非局部的goto或“长跳转”到由j描述的上下文处（也就是原来调用setjmp(j)处）。当作为长跳转的目标而被调用时，setjmp()返回r或1（如果r设为0的话）。（记住，setjmp()不能在这种情况时返回0）
通过有两类返回值，setjmp()让你知道它正在被怎么使用。当设置j时，setjmp()如你期望地执行；但当作为长跳转的目标时，setjmp()就从外面“唤醒”它的上下文。可以将longjmp()用来终止异常，用setjmp()标记相应的异常处理程序。
##信号（signal.h）
此头文件包含两个函数和一些宏定义。
核心是两个函数：

	void ( *signal( int signum, void (*handler)(int) ) )(int);
	或
	typedef void (*sig_t)(int);
	sig_t signal(int signum, sig_t handler);
	int raise(int signum);
###signal函数
signal函数用来设置某一信号的对应动作。
执行成功，返回旧handler的指针；否则，返回SIG_ERR。
第一个参数signum指明了所要处理的信号类型，应使用后面说明的信号编号宏定义。
第二个参数handler描述了与信号相关联的动作，它可以取以下三种值：
（1）一个无返回值的函数指针
此函数必须在signal函数被调用前申明，handler为这个函数的名字。当接收到一个编号为signum的信号时，就执行handler所指定的函数。这个函数应有如下形式的定义：void func(int sig);。
（2）SIG_IGN
这个符号表示忽略该信号。
（3）SIG_DFL
这个符号表示恢复系统对该信号的默认处理。
When signal handler is set to a function and a signal occurs, it is implementation defined whether signal(sig, SIG_DFL); will be executed immediately before the start of signal handler. Also, the implementation can prevent some implementation-defined set of signals from occurring while the signal handler runs.

raise函数用来把信号signum送给正在执行的程序。
执行成功，返回0；否则，返回非零。
###还有一些宏定义
	// signal return values
	#define	SIG_DFL	((void(*)(int)) 0)
	#define	SIG_IGN	((void(*)(int)) 1)
	#define	SIG_ERR	((void(*)(int)) -1)

	// signal codes
	#define	SIGINT		2 // Interactive attention
	#define	SIGILL		4 // Illegal instruction
	#define	SIGFPE		8 // Floating point error
	#define	SIGSEGV		11 // Segmentation violation
	#define	SIGTERM		15 // Termination request
	#define	SIGABRT		22 // Abnormal termination (abort)
信号编号
SIGINT，收到一个交互的提示信号，当用户按下Ctrl-C会产生一个SIGINT信号。
SIGILL，检测到无效的函数映像，比如一条非法指令。
SIGFPE，错误的算术操作，比如除零或者导致溢出的操作。
SIGSEGV，对存储器的无效访问，向一个空指针写数据会产生一个SIGSEGV信号。
SIGTERM，送到程序的终止请求。
SIGABRT，异常终止，比如执行了abort函数。
###注意
并不一定非要调用raise函数才会释放相应的信号，比如当按下ctrl-c时，并没有调用raise，但系统会产生一个SIGINT信号。

另外，此头文件还定义了sig_atomic_t这个类型，An integer type which can be accessed as an atomic entity even in the presence of asynchronous interrupts made by signals。
##可变参数表列（stdarg.h）
此头文件定义了三个宏和一个类型。
这个类型是va_list，它可能被实现为char*的typedef。
这三个宏都在拥有可变参数的函数定义里被调用，下面的ap是一个va_list类型的变量。
###va_start(ap,A)
A是参数表列里...之前的最后一个参数，执行完此语句，ap会指向A之后的下一个参数（也就是可变参数中的第一个参数）；
###av_arg(ap,T)
T是对应参数的类型，执行完此语句，ap会指向下一个参数；
###va_end(ap)
释放ap，不过在一些实现里，被定义成void(0)。

这个机制基于如下（但不仅限于）假设：
一个可变参数表在内存中占据了一个连续的字符数组；
后继的参数占据着字符数组更高位的后继元素；
2^N字节对齐；
参数表列里有一个指示参数数目的参数或有一个指示参数表列结束的标志。
##标准定义（stddef.h）
此头文件定义了3个类型和2个宏，其中一些在其他头文件中也有定义。

这3个类型是：
ptrdiff_t：两个指针相减的结果的类型，有符号整型，一般来说是int或long的typedef。
size_t：是sizeof操作符的结果的类型，无符号整型，一般来说定义成unsigned int或unsigned long，但应与ptrdiff_t相对应。
wchar_t：是一个整型，标识一个宽字节字符，例如`L'x'`的类型就是wchar_t。

这2个宏是：
NULL：空指针常量。
offsetof(type, member-designator)：展开为一个size_t类型的整值常量表达式，它的值是从结构的起始位置到结构成员的偏移量，以字节为单位。

本头文件一个可能的实现：

	#ifndef _STDDEF
	#define _STDDEF

	typedef unsigned int size_t;
	typedef unsigned short wchar_t;
	typedef int ptrdiff_t;
	#define NULL (void *)0
	#define offsetof(T, member) ((size_t)&((T *)0)->member)

	#endif // _STDDEF
##标准工具库（stdlib.h）
此头文件定义了4种类型和几个宏。
size_t和wchar_t在stddef.h里也有定义。
div_t是一个结构类型，是函数div的返回值的类型，ldiv_t也是一个结构类型，是函数ldiv的返回值类型。

定义的宏：
NULL。
EXIT_FAILURE，EXIT_SUCCESS，它们展开为整值常量表达式，可作为exit函数的参数使用，分别返回给宿主环境不成功和成功终止的状态。
RAND_MAX展开为整值常量表达式，其值是rand函数返回的最大值。
MB_CUR_MAX展开为整值常量表达式，其值是当前区域设置指定的扩展字符集中多字节字符的最大字节数目。

本文件还声明了诸多函数。分类如下：
	整型数学
执行简单的整型算术：abs，div，labs，ldiv
	算法
复杂而又被广泛使用、足以打包作为库函数的操作：bsearch，qsort，rand，srand
	文本转换
确定文本表示的编码算术值：atof，atoi，atol，strtod，strtol，strtoul
	多字节转换
多字节和宽字节字符编码之间的转换：mblen，mbstowcs，mbtowc，wcstombs，wctomb
	存储分配
管理数据对象的堆：calloc，free，malloc，realloc
	环境接口
程序和执行环境之间的接口：abort，atexit，exit，getenv，system
###生成随机数
	int rand(void);
返回一个伪随机整数，范围在0到RAND_MAX之间（包括0和RAND_MAX）。

	void srand(unsigned int seed);
以seed初始化随机数发生器，这个种子默认为1，也就是说如果在调用rand之前不调用srand，相当于调用了“srand(1);”。
###绝对值和整数除法
	int abs(int j);
	long int labs(long int j);
	div_t div(int number, int denom);
	ldiv_t ldiv(long int number, long int denom);
div_t有两个成员，int quot和int rem，分别用来保存number除以denom的商和余数。
相比div_t，ldiv_t的两个成员的类型只是都变成long int而已。
###内存管理
	void *malloc(size_t size);
为一个对象分配空间，该对象的大小是size。如果失败返回空指针。

	void free(void *ptr);
ptr如果是空指针，不发生任何行为；其他不适当的参数会导致未定义的行为。

	void *calloc(size_t nmemb, size_t size);
为拥有nmemb个元素的数组分配空间，每一个元素的大小是size，数组将被初始化为0。分配失败返回空指针。

	void *realloc(void *ptr, size_t size);
将ptr指向的对象的大小改变为由size指定的大小。如果ptr是一个空指针，与malloc行为相同；如果size为0且ptr不是空指针，与free行为相同。
###字符转换函数
	int atoi(const char *nptr);
除了出错后的行为，它等价于(int)strtol(nptr, (char **)NULL, 10);

	long int atol(const char *nptr);
除了出错后的行为，它等价于strtol(nptr, (char **)NULL, 10);

	double atof(const char *nptr);
把nptr指向的字符串的初始部分转换为double类型的表示。除了出错后的行为，它等价于strtod(nptr, (char **)NULL);

	long int strtol(const char *nptr, char **endptr, int base);
如果存在转换后的值，则返回这个值。如果没有执行转换，则返回0。如果正确的转换值在可表示的范围值外，则返回LONG_MAX或者LONG_MIN，并将宏ERANGE的值存储在errno中。

	unsigned long int strtoul(const char *nptr, char **endptr, int base);
如果存在转换后的值，则返回这个值。如果没有执行转换，则返回0。如果正确的转换值在可表示的范围值外，则返回ULONG_MAX，并将宏ERANGE的值存储在errno中。

	double strtod(const char *nptr, char **endptr);
它把输入串分解为3个部分：可能为空的初始序列，由一些空白字符组成（由isspace函数指定）；目标序列；一个或多个不能识别的字符组成的序列，包括输入串结尾的空字符。目标序列的期望形式是：-5.3e-3，起始字符可以是正负符号，可以是0-9，可以是小数点。
如果存在转换后的值，则返回这个值。如果没有执行转换，则返回0。如果正确的转换值在可表示的范围值外，则返回正或者负的HUGE_VAL，并将宏ERANGE的值存储在errno中。如果正确的转换值会造成下溢，则返回0，并将宏ERANGE的值存储在errno中。
如果endptr不为空，则指向最后串的指针存储在endptr指向的对象中。

附函数strtol可能的实现代码：
```
#include <errno.h>
#include <limits.h>
int isspace(int c)
{
    if(c >=9 && c <= 12 || c == 32) return 1;
    return 0;
}
int tolower(int c)
{
    if(c >= 65 && c <= 90) c += 32;
    return c;
}

#define BASE_MAX 36 // 可接受的最大进制
static const char digits[] =
{"0123456789abcdefghijklmnopqrstuvwxyz"};

long strtol(const char *s, char **endptr, int base)
{
    const char *sc = s;
    long y, x = 0; // 临时变量

    while(isspace(*sc)) ++sc; // 跳过空白

    char sign = *sc == '-' || *sc == '+' ? *sc++ : '+'; // 记录符号

    if(base < 2 || BASE_MAX < base) // 检查进制
    {
        if(endptr) *endptr = (char *)s;
        return 0;
    }
    else if(*sc == '0' && (sc[1] == 'x' || sc[1] == 'X')) // 跳过16进制下开头的0x
    {
        if(base != 16)
        {
            if(endptr) *endptr = (char *)s;
            return 0;
        }
        sc += 2;
    }

    for(; *sc == '0'; ++sc); // 跳过0

    for(const char *sd; (sd = (const char *)memchr(digits, tolower(*sc), base)) != NULL; ++sc)
    {
        y = x; // 记录本次x
        x = x * base + (sd - digits);
    }

    ptrdiff_t rem = (const char *)memchr(digits, tolower(*(sc-1)), base) - digits;
    if((x - rem) / base != y) // 测试是否溢出
    {
        errno = ERANGE;
        if(sign == '+') x = LONG_MAX;
        else x = LONG_MIN, sign = '+';
    }

    if(sign == '-') x = -x;
    if(endptr) *endptr = (char *)sc;

    return x;
}
```
###查找和排序
	void qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));
根据compare所指向的比较函数将数组内容排列成升序。如果第一个元素小于、等于或大于第二个元素，compare分别返回小于0，等于0和大于0的整数。
数组的大小为nmemb，每个元素的大小为size，base指向第一个元素。

	void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));
搜索一个拥有nmemb个元素的数组，来查找与key指向的对象匹配的元素，base指向这个数组的第一个元素。数组中的每个元素的大小由size指定。
如果有多个元素命中，可能返回指向任一个的指针。
compare的第一个参数指向key数据，第二个参数指向一个数组元素。
要保证数组必须是非降序排列的。
###环境通信函数
	int atexit(void (*func)(void));
注册func指向的函数，该函数在程序正常终止的时候被调用。成功返回0，否则返回非0。实现应该至少支持32个函数的注册，一个函数可以注册多次。

	void exit(int status);
使程序正常终止。
首先，调用所有atexit注册的函数，按照他们注册时的反顺序调用。
然后，清空所有打开的具有未写缓冲数据的流，关闭所有打开的流，并删除tmpfile函数创建的所有文件。
最后，控制权返回给宿主环境。

	void abort(void);
使程序异常终止，除非捕获了信号SIGABRT且信号处理程序没有返回。可能会实现为调用exit函数并释放SIGABRT信号。

	char *getenv(const char *name);
搜索宿主环境提供的环境表，来查找一个能和name指向的串匹配的串。如果不能找到，则返回空指针。也就是windows下的环境变量。

	int system(const char *string);
把sring指向的串传递给宿主环境，然后命令处理程序按照实现定义的方式执行它，可以使用空指针作为参数查询命令处理程序是否存在。如果参数是空指针，只有当命令处理程序可用的时候返回非0。如果参数不是空指针，返回一个实现定义的值。
###多字节字符和多字节串函数
这些函数的行为受当前区域设置类别LC_CTYPE的影响，在此仅列出：

	int mblen(const char *s, size_t n);
	int mbtowc(wchar_t *pwc, const char *s, size_t n);
	int wctomb(char *s, wchar_t wchar);
	size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
	size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);