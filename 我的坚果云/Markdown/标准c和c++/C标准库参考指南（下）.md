下节包括stdio.h，string.h，time.h和另外一些东西。
上节链接 http://blog.csdn.net/liuyuan185442111/article/details/43673461
##输入输出流（stdio.h）
此头文件最常用，也很复杂。它包含3种类型、一些宏和很多的执行输入输出的函数。

声明的类型有size_t，FILE，fpos_t。
FILE是一个结构体类型，记录了控制流需要的所有信息，包括它的文件定位符、指向相关缓冲的指针、记录是否发生了读/写错误的错误提示符和记录文件是否结束的文件结束符。
fpos_t包含可以唯一指定文件中的每一个位置所需的所有信息。

定义的宏：
NULL。
_IOFBF、_IOLBF、_IONBF：它们展开为具有不同值的整值常量表达式，适合作为函数setvbuf的第三个参数使用。
BUFSIZ：整值常量表达式，指setbuf函数使用的缓冲的大小。
EOF：负的整值常量表达式，该表达式由几个函数返回来说明文件的结束，即一个流输入结束了。
FOPEN_MAX：表示实现支持的可以同时打开的文件数目的最小值。
FILENAME_MAX：表示一个char类型数组的大小，这个数组可以保存实现可以打开的最长的文件名串。
L_tmpnam：表示一个char类型数组的大小，这个数组可以保存tmpnam函数生成的临时文件名串。
SEEK_CUR、SEEK_END、SEEK_SET：适合作为fseek函数的第三个参数使用。
TMP_MAX：表示tmpnam函数可以生成的单独文件名的最大数目。
stderr、stdin、stdout：它们是指向FILE的指针类型的表达式，分别指向标准错误流、标准输入流、标准输出流相关的FILE对象。
###对文件的操作x4
	int remove(const char *filename);
删除文件。成功返回0，否则返回非0。

	int rename(const char *old, const char *new);
重命名文件。成功返回0，失败返回非0。

	FILE *tmpfile(void);
创建一个临时的二进制文件，并通过模式“wb+”打开。当这个文件关闭或者程序终止的时候它会被自动删除。失败返回空指针。

	char *tmpname（char *s）;
生成一个字符串，这个字符串是一个有效的文件名，并且它不和现有的文件名相同。每调用一次就生成一个不同的名字，最多可调用TMP_MAX次。
如果参数是一个空指针，把结果保存在一个内部的静态对象中，并返回指向该对象的指针。如果参数不是一个空指针，就认为它指向一个至少有L_tmpnam个字符元素的数组，函数把结果写到该数组中并返回参数值。
###文件访问函数x6
	FILE* fopen(const char *filename, const char *mode);
打开名字为filename指向的串的文件，并且把这个文件和一个流相关联，返回指向控制流的对象的指针。如果打开操作失败，返回空指针。

	FILE* freopen(const char *filename, const char *mode, FILE *stream);
打开名字为filename指向的串的文件，并且把它和stream指向的流关联在一起。首先尝试关闭和指定的流相关联的任意文件，如果不能成功关闭，则忽略这一点，然后清空流的错误提示符和文件结束符。如果打开文件操作失败，返回空指针，否则返回stream的值。

	int fclose(FILE *stream);
使stream指向的流被清空，并且和流相关联的文件被关闭。如果成功，返回0；否则返回EOF。

	int fflush(FILE *stream);
对stream指向的流执行清空行为，如果stream是空指针，则对所有的流执行清空行为。如果发生写入错误，返回EOF；否则返回0。

	void setbuf(FILE *stream, char *buf);
除了没有返回值，等价于函数setvbuf，而该函数是以参数mode为_IOFBF的值和参数size为BUFSIZ的值被调用的，或者（如果buf是一个空指针）是以参数mode为_IONBF的值被调用的。

	void setvbuf(FILE *stream, char *buf, int mode, size_t size);
只能在stream指向的流和一个打开文件相关联之后，而且还没有对流执行任何其他操作之前使用。参数mode决定了stream缓冲的方式：_IOFBF导致输入/输出完全缓冲；_IOLBF导致输入/输出行缓冲；_IONBF导致输入/输出不缓冲。如果buf不是一个空指针，可以使用它指向的数组来代替setvbuf函数分配的缓冲区，size指定了数组的大小。
成功返回0，如果赋予参数mode一个无效的值或请求没有被响应，返回非0。
###格式化的输入输出函数x9
	int fprintf(FILE *stream, const char *format, ...);
	int printf(const char *format, ...);
	int sprintf(char *s, const char *format, ...);
返回传送的字符的数目，如果发生了输出错误，就返回一个负值。如果写入数组，在写入的字符最后写入一个空字符，它不计入返回和的一部分。

	int fscanf(FILE *stream, const char *format, ...);
	int scanf(const char *format, ...);
	int sscanf(char *s, const char *format, ...);
如果在任何转换之前发生了输入失败，返回EOF；否则返回赋值的输入项的数目。

	int vfprintf(FILE *stream, const char *format, va_list arg);
	int vprintf(const char *format, va_list arg);
	int vsprintf(char *s, const char *format, va_list arg);
等价于对应的printf函数，不过可变参数表用arg代替，它已经被宏va_start初始化了。函数并没有调用va_end，所以需要使用者自行调用。
下面的例子展示了常规的错误报告中对vfprintf的使用：
```
void error(const char *name, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERROR in %s: ", name);
    vfprintf(stderr, format, args);
    va_end(args);
}
```
###字符输入/输出函数x11
	int fgetc(FILE *stream);
从stream指向的输入流中读取下一个字符（如果有的话），并把它由unsigned char转换为int类型，并且流的相关的文件定位符（如果定义的话）向前移动一位。返回stream指向的输入流中的下一个字符。如果输入流在文件结束处，则设置文件结束符，返回EOF。如果发生了读错误，则设置流的错误指示符，函数返回EOF。

	int getc(FILE *stream);
等价于函数fgetc，不过getc作为一个宏实现，它可能会对stream进行多次计算，所以它的参数不能是具有副作用的表达式。

	int getchar(void);
等价于用参数stdin调用函数getc。

	int fputc(int c, FILE *stream);
把c指定的字符（转换为unsigned char类型）写到stream指向的输出流中相关的文件定位符（如果定义的话）指定的位置处，并把文件定位符向前移动适当的位置。如果文件不支持定位要求或者流以附加模式打开，字符就添加到输出流的后面。返回写入的字符。如果发生了错误，则设置流的错误指示符，返回EOF。

	int putc(int c, FILE *stream);
等价于fputc，不过getc作为一个宏实现，它可能会对stream进行多次计算，所以它的参数不能是具有副作用的表达式。

	int putchar(int c);
等价于把stdout作为第二个参数调用的putc。

	char *fgets(char *s, int n, FILE *stream);
从stream指向的流中读取字符，读取字符的数目最多比n指定的数目少1，然后将字符写入到s指定的数组中。读入换行符（保留）或者文件结束之后，不再读取其他字符。最后一个字符写入数组之后立即写入一个空字符。如果成功执行，返回s。如果遇到文件结束且没有向数组中写入字符，则数组的内容不变且返回一个空指针。如果操作的过程中发生了读错误，则数组的内容不确定，并返回一个空指针。

	int fputs(const char *s, FILE *stream);
把s指向的串写入stream指向的流中，不写入结束的空字符。如果发生了写错误，返回EOF；否则，返回一个非负值。

	char *gets(char *s);
从stdin指向的输入流中读取若干个字符，并将其保留到s指向的数组中，直到遇到文件结束符或者读取一个换行符。所有的换行符都被丢弃，在最后一个字符读到数组中之后立即写入一个空字符。成功执行时返回s，如果遇到文件结束符且数组中没有读入任何字符，则数组的内容不变，返回一个空指针。如果操作的过程中发生了错误，则数组的内容不变，返回一个空指针。

	int puts(const char *s);
把s指向的串写到stdout指向的流中，并且在输出最后添加一个换行符。不写入结束的空字符。如果发生了写错误，返回EOF，否则返回一个非负值。

	int ungetc(int c, FILE *stream);
把c指定的字符（转换为unsigned char类型）退回到stream指向的输入流中。返回转换后的回退字符，如果操作失败，返回EOF。
###直接输入/输出函数x2
	size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
从stream指向的流中读取最多nmemb个元素到ptr指向的数组中，元素大小是由size指定的。返回成功读取的元素的数组，如果发生了读错误或者遇到文件结束，返回值可能比nmemb小。如果size或nmemb为0，则fread返回0且数组的内容和流的状态保持不变。

	size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
从ptr指向的数组中读取最多nmemb个元素并将其写到stream指向的流中，元素大小是由size指定的。返回成功写入的元素的数目，如果遇到写错误，可能比nmemb小。
###文件定位函数x5
	int fgetpos(FILE *stream, fpos_t *pos);
把stream指向的流的文件定位符的当前值存储到pos指向的对象中。存储的值包含了未指明的信息，函数fsetpos可以使用这些信息来使流重新定位到这次调用函数fgetpos时的位置。如果操作成功，返回0；否则返回非0并且把一个由实现定义的正值存储在errno中。

	int fsetpos(FILE *stream, const fpos_t *pos);
根据pos指向的对象的值来设置stream指向的流的文件定位符，pos指向的对象的值应该是前面对同一个流调用函数fgetpos得到的返回值。成功调用会清空流的文件结束符并且解除ungetc对这个流的影响。如果操作成功，返回0；否则返回非0并且把一个由实现定义的正值存储在errno中。

	long int ftell(FILE *stream);
获得stream指向的流的文件定位符的当前值。对一个二进制流来说，这个值是从文件开头到当前位置的字符的数目；对一个文本流来说，它的文件定位符包含了未说明的信息，函数fseek可以使用这些信息来使流的文件定位符回到这次调用ftell时的位置。如果成功执行，返回流的文件定位符的当前值，如果失败，返回-1L并且把一个实现定义的正值存储在errno中。

	int fseek(FILE *stream, long int offset, int whence);
为stream指向的流设置文件定位符。只有当不能满足请求时，才返回非0值。对二进制流来说，新的位置从文件的开始以字符为单位进行测量，通过在whence指定的位置加上offset来获得。如果whence是SEEK_SET，指定的位置就是文件的开始位置；如果是SEEK_CUR，就是文件的当前位置；如果是SEEK_END，则是文件的结束位置。
对文本流来说，offset或者是0，或者前面对同一个流调用ftell函数返回的值，whence应该是SEEK_SET。
成功调用会清空流的文件结束符并且解除ungetc对这个流的影响。

	void rewind(FILE *stream);
把stream指向的流的文件定位符设置在文件的开始位置，等价于(void)fseek(stream, 0L, SEEK_SET);，只不过流的错误指示符也被清零。
###错误处理函数x4
	void clearerr(FILE *stream);
清空stream指向的流的文件结束符和错误指示符。

	int feof(FILE *stream);
测试stream指向的流的文件结束符。当且仅当stream流设置了文件结束符时函数返回一个非0值。

	int ferror(FILE *stream);
测试stream指向的流的错误指示符。当且仅当stream流设置了错误指示符时函数返回一个非0值。

	void perror(const char *s);
把整数表达式errno中的错误编号转换为一条错误消息。它把一个字符序列写到标准错误流：如果s不是空指针且s指向的字符不是空字符，s指向的串后面跟一个冒号和一个空格；然后是一个适当的错误信息串后面跟一个换行符。错误信息串的内容和用参数errno调用函数strerror的返回值相同。
###fopen函数的mode参数
r	打开现有文本文件以便读取
w	生成新文本文件或截短现有文本文件至零长度以便写入
a	附加。生成新文本文件或打开现有文本文件以便在文件结束处写入
rb	打开现有二进制文件以便读取
wb	生成新二进制文件或截短现有二进制文件至零长度以便写入
ab	附加。生成新二进制文件或打开现有二进制文件以便在文件结束处写入
r+	打开现有文本文件，以便更新（读和写）
w+ 生成新文本文件或截短现有文本文件至零长度以便更新
a+	附加。生成新文本文件或打开现有文本文件以便更新，在文件结束符处写入
r+b/rb+	打开现有二进制文件，以便更新（读和写）
w+b/wb+	生成新二进制文件或截短现有二进制文件至零长度以便更新
a+b/ab+	附加。生成新二进制文件或截短现有二进制文件至零长度以便更新，在文件结束符处写入

用读模式打开一个文件时（r作为mode参数的第一个字符），如果文件不存在或不能读，操作就会失败。
##字符串（string.h）
此头文件包含一种类型和几个函数，并定义了一个宏。声明的类型是size_t，定义的宏是NULL。

名字以mem开头的函数对任意的字符序列进行操作。其中一个参数指向字符串的起始位置，另一个参数对元素的个数尽行计数。
名字以strn开头的函数对非空字符序列进行操作。
名字以str开头的函数对空字符结尾的字符序列进行操作。
###mem类x5
	void *memmove(void *s1, const void *s2, size_t n);
	void *memcpy(void *s1, const void *s2, size_t n);
memmove和memcpy均从s2指向的对象中复制n个字符到s1指向的对象中，返回s1的值。不同之处在于，当复制发生在两个重叠的对象中，memcpy对这种行为未定义，memmove则可以正确执行。

	int memcmp(const void *s1, const void *s2, size_t n);
比较s1和s2指向的对象中前n个字符。

	void *memchr(const void *s, int c, size_t n);
返回指向定位的字符的指针，如果没有返回空指针。

	void *memset(void *s, int c, size_t n);
把c的值复制到s指向的对象的前n个字符的每个字符中，返回s的值。
###str类x6+3+1
```
char *strcpy(char *s1, const char *s2);
包括终止的空字符，返回s1的值
char *strcat(char *s1, const char *s2);
包括终止的空字符，返回s1的值。
int strcmp(const char *s1, const char *s2);
比较s1和s2指向的串。
char *strchr(const char *s, int c);
终止的空字符被认为串的一部分。
char *strrchr(cosnt char *s, int c);
确定c在s指向的串中最后一次出现的位置。r意为right一侧。
char *strstr(const char *s1, const char *s2);
s2指向的串的字符序列在s1指向的串中第一次出现的位置。

size_t strspn(const char *s1, const char *s2);
计算s1指向的字符串中完全由s2指向的字符串中的字符组成的最大初始段的长度。
也就是说，它从s1的开头搜索一个和s2中的任意元素都不匹配的字符，返回此字符的下标。
size_t strcspn(const char *s1, const char *s2);
查找两个字符串第一个相同的字符在s1中的位置，也就是The first char both in s1 and s2，
如果没有则返回终止的空字符的索引。
strcspn和strchr很相似，但它匹配的是任意一个字符集而不是一个字符。
char *strpbrk(const char *s1, const char *s2);
确定s2指向的串中的任意的字符在s1中第一次出现的位置。
似乎，strcspn和strpbrk功用相同，仅返回值不同。

char *strtok(char *s1, const char *s2);
分解字符串为一组字符串。s1为要分解的字符串，s2为分隔符字符串。
首次调用时，s1指向要分解的字符串，之后再次调用要把s1设成NULL。
如果待查找的字符串里没有s2指定的分隔符，函数返回NULL。
注意，此函数会修改s1指向的串，返回的也是原串的一部分，如做他用，或许需要再复制一份。
```
###strn类x3
```
char *strncpy(char *s1, const char *s2, size_t n);
从s2指向的数组中复制最多n个字符（包括空字符），不会在s1的末尾自动添加空字符；
如果达到n个字符之前遇到了空字符，则复制完空字符后停止，
并在s1指向的数组后面添加空字符，直到写入了n个字符。
简单来说，复制，直到遇到空字符或达到n个字符，如果此时复制的字符数未达到n，
就填充空字符，直到字符数达到n。
char *strncat(char *s1, const char *s2, size_t n);
从s2指向的数组中将最多n个字符（空字符及其后面的字符不添加）添加到s1指向的串的结尾。
在最后的结果后面加上一个空字符。
简单来说，cat，直到遇到空字符或达到n个字符，最后的结果一定以空字符结尾。
int strncmp(const char *s1, const char *s2, size_t n);
对s1和s2指向的数组中的最多n个字符进行比较（空字符后面的字符不参加比较）。
简单来说，比较，直到达到n个字符，或者已得到结果。
```
###其他类
	size_t strlen(const char *s);
计算s指向的串的长度，不包括结尾的空字符。

	char *strerror(int errnum);
将errnum中的错误编号对应到一个错误信息串。
###与区域设置有关
	int strcoll(const char *s1, const char *s2);
功能同strcmp，只是比较时串都被解释为适合当前区域设置的类别LC_COLLATE的形式。

	size_t strxfrm(char *s1, const char *s2, size_t n);
如果区域选项是“POSIX”或者“C”，那么strxfrm()同用strncpy()来拷贝字符串是等价的。
##时间（time.h）
此头文件定义了两个宏，四种类型和几个操作时间的函数。
定义的宏有NULL和CLOCKS_PER_SEC，后者使“clock函数的返回值/CLOCKS_PER_SEC”的单位是秒。实现可能将其定义为1000，这样clock函数返回的就是毫秒，但也可能被定义为其他值。

声明的类型有size_t、clock_t、time_t和struct tm。
size_t在stddef.h中已介绍过。
clock_t是clock函数的返回值类型，time_t是time函数的返回值类型，二者都是有符号整型。
struct tm保存了一个日历时间的各组成部分，比如年月日时分秒等。
###时间获取函数
	time_t time(time_t *timer);
获取时间戳，如果参数不是NULL，返回值同时赋给它指向的对象，失败返回-1。

	clock_t clock(void);
确定处理器使用的时间。如果需要以秒计数，只需将返回值除以CLOCKS_PER_SEC。失败返回-1。
###时间转换函数
下表描述了三种时间表达形式间的相互转换。time_t代表时间戳，地球上的任意时区这个值都相同且唯一。struct tm代表日历时间，其元素包含年月日时分秒星期等信息，其值与时区有关，0时区时间转换到北京东八区时间，要加上8小时。char*代表用字符串表示的时间。
source|destination|function
-|-|-
time_t|struct tm（0时区）|gmtime
time_t|struct tm（本地时区）|localtime
time_t|char*（本地）|ctime
struct tm（本地）|time_t|mktime
struct tm|char*|asctime/strftime
这些转换函数的声明：

	struct tm *gmtime(const time_t *timer);
	struct tm *localtime(const time_t *timer);
	char *ctime(const time_t *timer);
	time_t mktime(struct tm *timeptr);
	char *asctime(const struct tm *timeptr);
	size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr);

ctime函数返回的字符串是这种形式的“Mon Oct 13 18:43:43 2014\n”。
ctime(t);等价于asctime(localtime(t));
###asctime/strftime
asctime返回的字符串形式是固定的，strftime返回的字符串形式可以自定义。详见原书。
###其他函数
double difftime(time_t time1, time_t time0);
计算两个日历时间之差：time1-time0，返回以秒为单位的差值。
##其他
###关于NULL和size_t
根据c89，NULL和size_t在多个头文件中都有定义，二者均在stddef.h、stdlib.h、stdio.h、string.h中有定义，除此之外NULL还在locale.h和time.h中有定义。
不过，在有需要的时候，还是包含stddef.h的好（我的猜测）。
###string.h可能的部分实现
```
#include <stddef.h>

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char uc = c;
    const unsigned char *su = s;
    for(; 0 < n; ++su, --n)
        if(*su == uc)
            return (void *)su;
    return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *su1 = s1, *su2 = s2;
    for(; 0 < n; ++su1, ++su2, --n)
        if(*su1 != *su2)
            return (*su1 < *su2) ? -1 : +1;
    return 0;
}

void *memset(void *s, int c, size_t n)
{
    const unsigned char uc = c;
    unsigned char *su = s;
    for(; 0 < n; ++su, --n)
        *su = uc;
    return s;
}

void *memcpy(void *s1, const void *s2, size_t n)
{
    char *su1 = s1;
    const char *su2 = s2;
    for(; 0 < n; ++su1, ++su2, --n)
        *su1 = *su2;
    return s1;
}

void *memmove(void *s1, const void *s2, size_t n)
{
    char *sc1 = s1;
    const char *sc2 = s2;
    if(sc2 < sc1 &&  sc1 < sc2 + n)
        for(sc1 += n, sc2 += n; 0 < n; --n)
            *--sc1 = *--sc2;
    else
        for(; 0 < n; --n)
            *sc1++ = *sc2++;
    return s1;
}

char* strchr(const char *s, int c)
{
    const char ch = c;
    for(; *s != ch; ++s)
        if(*s == '\0')
            return NULL;
    return (char *)s;
}

char *strstr(const char *s1, const char *s2)
{
    if(*s2 == '\0')
        return (char *)s1;
    for(; (s1 = strchr(s1, *s2)) != NULL; ++s1)
    {
        const char * sc1 = s1, *sc2 = s2;
        while(1)
            if(*++sc2 == '\0')
                return (char *)s1;
            else if(*++sc1 != *sc2)
                break;
    }
    return NULL;
}

char *strpbrk(const char *s1, const char *s2)
{
    const char *sc1, *sc2;

    for(sc1 = s1; *sc1 != '\0'; ++sc1)
        for(sc2 = s2; *sc2 != '\0'; ++sc2)
            if(*sc1 == *sc2)
                return (char *)sc1;
    return NULL;
}

size_t strspn(const char *s1, const char *s2)
{
    const char *sc1, *sc2;

    for(sc1 = s1; *sc1 != '\0'; ++sc1)
        for(sc2 = s2; ; ++sc2)
            if(*sc2 == '\0')
                return sc1 - s1;
            else if(*sc1 == *sc2)
                break;
    return sc1 - s1;
}

size_t strcspn(const char *s1, const char *s2)
{
    const char *sc1, *sc2;

    for(sc1 = s1; *sc1 != '\0'; ++sc1)
        for(sc2 = s2; *sc2 != '\0'; ++sc2)
            if(*sc1 == *sc2)
                return sc1 - s1;
    return sc1 - s1;
}

char *strtok(char *s1, const char *s2)
{
    char *sbegin, *send;
    static char *ssave = "";

    sbegin = s1 ? s1 : ssave;
    sbegin += strspn(sbegin, s2);
    if(*sbegin == '\0')
    {
        ssave = "";
        return NULL;
    }
    send = sbegin + strcspn(sbegin, s2);
    if(*send != '\0')
        *send++ = '\0';
    ssave = send;
    return sbegin;
}

#include <stdio.h>
int main()
{
    putchar(*(char*)memchr("abc", 'a', 3));
    puts("");
    char ip[] = "192.168.199.122";
    puts(strtok(ip,"."));
    puts(strtok(NULL,"."));
    puts(strtok(NULL,"."));
    puts(strtok(NULL,"."));
    return 0;
}
```