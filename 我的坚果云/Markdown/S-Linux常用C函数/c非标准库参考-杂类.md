##calloc
配置内存空间

	#include <stdlib.h>
	void *calloc(size_t n, size_t size);
calloc()用来配置n个相邻的内存单位，每一单位的大小为size，并返回指向第一个元素的指针。这和使用下列的方式效果相同：malloc(n*size);，不过，在利用calloc()配置内存时会将内存内容初始化为0。

若配置成功则返回一指针，失败则返回NULL。
##fdopen
将文件描述符转为文件指针

	#include <stdio.h>
	FILE *fdopen(int fd, const char *mode);
fdopen()会将参数fd的文件描述符，转换为对应的文件指针后返回。参数mode字符串则代表着文件指针的流形态，此形态必须和原先文件描述符读写模式相同。

转换成功时返回指向该流的文件指针。失败则返回NULL，并把错误代码存在errno中。
```
#include <stdio.h>
int main()
{
	FILE *fp = fdopen(1, "w+");
	fprintf(fp, "%s\n", "hello!");
	fclose(fp);
	return 0;
}

hello!

但是如果把输出重定向到文件的话，则会报段错误，为何？
```
##fileno
返回文件流所使用的文件描述符

	#include <stdio.h>
	int fileno(FILE *stream);

fileno()用来取得参数stream指定的文件流所使用的文件描述符。

返回文件描述符。
```
#include <stdio.h>
int main()
{
	FILE *fp;
	int fd;
	fp=fopen("/etc/passwd", "r");
	fd=fileno(fp);
	printf("fd=%d\n", fd);
	fclose(fp);
	return 0;
}

fd=3
```
##gcvt
将浮点型数转换为字符串，取四舍五入

	#include <stdlib.h>
	char *gcvt(double number, size_t ndigits, char *buf);
gcvt()用来将参数number转换成ASCII码字符串，参数ndigits表示显示的数字的数目。若转换成功，转换后的字符串会放在参数buf指针所指的空间。

返回一字符串指针，此地址即为buf指针。
##getopt
分析命令行参数

	#include <unistd.h>
	extern char *optarg;
	extern int optind, opterr, optopt;
	int getopt(int argc, char *const argv[], const char *optstring);
参数argc和argv是由main()传递的参数个数和内容。参数optstring则代表欲处理的选项字符串。此函数会返回在argv中下一个的选项字母，此字母会对应参数optstring中的字母。如果选项字符串里的字母后接着冒号“:”，则表示还有相关的参数，全局变量optarg即会指向此额外参数。如果getopt()找不到符合的参数则会印出错信息，返回值为“?”字符，并将optopt设为选项字符，如果不希望getopt()打印出错信息，则只要将全局变量opterr设为0即可。

如果找到符合的参数则返回此参数字母，如果参数不包含在参数optstring的选项字母则返回“?”字符，分析结束则返回-1。
##getpagesize
取得内存分页大小

	#include <unistd.h>
	size_t getpagesize(void);
返回一分页的大小，单位为字节（byte）。此为系统的分页大小，不一定会和硬件分页大小相同。
在Intel x86上其返回值应为4096bytes。
##gettimeofday
取得当前的时间

	#include <sys/time.h>
	int gettimeofday(struct timeval *tv, struct timezone *tz);
gettimeofday()会把目前的时间由tv所指的结构返回，当地时区的信息则放到tz所指的结构中。
timeval结构定义为：

	struct timeval{
		long tv_sec; /*秒,也就是时间戳*/
		long tv_usec; /*微秒*/
	};
timezone 结构定义为:

	struct timezone{
		int tz_minuteswest; /*和Greenwich时间差了多少分钟*/
		int tz_dsttime; /*夏令时*/
	};
上述两个结构都定义在/usr/include/sys/time.h
如果tz是NULL，tz不会被填充。

成功则返回0，失败返回-1，错误代码存于errno。EFAULT：指针tv和tz所指的内存空间超出存取权限。

北京东八区tz->tz_minuteswest为-480，tz->tz_dsttime为0。
##settimeofday
设置当前时间

	#include <sys/time.h>
	#include <unistd.h>
	int settimeofday(const struct timeval *tv, const struct timezone *tz);
settimeofday()会把目前时间设成由tv所指的结构信息，当地时区信息则设成tz所指的结构。注意，只有root权限才能使用此函数修改时间。

成功则返回0，失败返回-1，错误代码存于errno。
EPERM：权限不够。
EINVAL：时区或某个数据是不正确的，无法正确设置时间。
##isatty
判断文件描述符是否是为终端机

	#include <unistd.h>
	int isatty(int desc);
如果参数desc所代表的文件描述词为一终端机则返回1，否则返回0。
##ttyname
返回一终端机名称，将文件描述符转化为文件名字符串

	#include <unistd.h>
	char *ttyname(int desc);
如果参数desc所代表的文件描述符为一终端机，则会将此终端机名称由一字符串指针返回，否则返回NULL。
```
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
	int fd;
	char *file = "/dev/tty";
	fd = open(fiel, O_RDONLY);
	printf("%s", file);
	if(isatty(fd))
	{
		printf("is a tty\n");
		printf("ttyname = %s\n", ttyname(fd));
	}
	else printf(" is not a tty\n");
	close(fd);
	return 0;
}

/dev/tty is a tty
ttyname = /dev/tty
```
##memccpy
拷贝内存内容

	#include <string.h>
	void *memccpy(void *dest, const void *src, int c, size_t n);
memccpy()用来拷贝src所指内存内容到dest所指的地址上，最多拷贝n个字符，当发现c时停止拷贝，返回c所在位置的下一个字节的地址，当没有发现c时返回NULL。
##mmap
建立内存映射

	#include <unistd.h>
	#include <sys/mman.h>
	void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);
mmap()用来将某个文件内容映射到内存中，对该内存区域的存取即是直接对该文件内容的读写。参数start指向欲对应的内存起始地址，通常设为NULL，表示让系统自动选定地址，对应成功后该地址会返回。参数length代表将文件中多大的部分对应到内存。
参数prot代表映射区域的保护方式有下列组合：
PROT_EXEC 映射区域可被执行
PROT_READ 映射区域可被读取
PROT_WRITE 映射区域可被写入
PROT_NONE 映射区域不能存取
参数flags会影响映射区域的各种特性：
MAP_FIXED 如果参数start所指的地址无法成功建立映射时，则放弃映射，不对地址做修正。通常不鼓励用此flag。
MAP_SHARED 对映射区域的写入数据会复制回文件内，而且允许其他映射该文件的进程共享。
MAP_PRIVATE 对映射区域的写入操作会产生一个映射文件的复制，即私人的“写入时复制”（copy on write）对此区域作的任何修改都不会写回原来的文件内容。
MAP_ANONYMOUS 建立匿名映射。此时会忽略参数fd，不涉及文件，而且映射区域无法和其他进程共享。
MAP_DENYWRITE 只允许对映射区域的写入操作，其他对文件直接写入的操作将会被拒绝。
MAP_LOCKED 将映射区域锁定住，这表示该区域不会被置换（swap）。
在调用mmap()时必须要指定MAP_SHARED或MAP_PRIVATE。参数fd为open()返回的文件描述词，代表欲映射到内存的文件。参数offset为文件映射的偏移量，通常设置为0，代表从文件最前方开始对应。

若映射成功则返回映射区的内存起始地址，否则返回MAP_FAILED(-1)，错误原因存于errno中。
EBADF：参数fd不是有效的文件描述词
EACCES：存取权限有误。如果是MAP_PRIVATE情况下文件必须可读，使用MAP_SHARED则要有PROT_WRITE。
EINVAL：参数start、length 或offset有一个不合法。
EAGAIN：文件被锁住，或是有太多内存被锁住。
ENOMEM：内存不足。
##munmap
解除内存映射

	#include <unistd.h>
	#include <sys/mman.h>
	int munmap(void *start, size_t length);
munmap()用来取消参数start所指的映射内存起始地址，参数length则是欲取消的内存大小。当进程结束或利用exec相关函数来执行其他程序时，映射内存会自动解除，但关闭对应的文件描述词时不会解除映射。

如果解除映射成功则返回0，否则返回-1，错误原因存于errno中。
##putenv
改变或增加环境变量

	#include <stdlib.h>
	int putenv(const char *string);
参数string的格式为name=value，如果该环境变量原先存在，则变量内容会依参数string改变，否则此参数内容会成为新的环境变量。

执行成功则返回0，有错误发生则返回-1。
ENOMEM：内存不足，无法配置新的环境变量空间。
```
#include <stdlib.h>
int main()
{
	char *p;
	if((p = getenv("USER")))
		printf("USER = %s\n", p);
	putenv("USER=test");
	printf("USER = s\n”, getenv("USER"));
}

USER = root
USER = test
```
##setenv
改变或增加环境变量

	#include <stdlib.h>
	int setenv(const char *name, const char *value, int overwrite);
参数name为环境变量名称字符串。参数value则为变量内容，参数overwrite用来决定是否要改变已存在的环境变量。如果overwrite不为0，而该环境变量原已有内容，则原内容会被改为参数value所指的变量内容。如果overwrite为0，且该环境变量已有内容，则参数value会被忽略。

执行成功则返回0，有错误发生时返回-1。
ENOMEM：内存不足，无法配置新的环境变量空间。
```
#include <stdlib.h>
int main()
{
	char *p;
	if((p=getenv("USER")))
		printf("USER = %s\n", p);
	setenv("USER", "test", 1);
	printf("USER = %s\n", getenv("USER"));
	unsetenv("USER");
	printf("USER = %s\n", getenv("USER"));
	return 0;
}

USER = root
USER = test
USER = (null)
```
##unsetenv

	#include <stdlib.h>
	int unsetenv(const char *name); 

The unsetenv() function deletes the variable name from the environment. If name does not exist in the environment, then the function  succeeds, and the environment is unchanged.

Return zero on success, or -1 on  error,  with errno set to indicate the cause of the error.
##strcasecmp
忽略大小写比较字符串

	#include <string.h>
	int strcasecmp(const char *s1, const char *s2);
##strncasecmp
忽略大小写比较字符串

	#include <string.h>
	int strncasecmp(const char *s1, const char *s2, size_t n);
strncasecmp()用来比较参数s1和s2字符串前n个字符，比较时会自动忽略大小写的差异。
##strdup
复制字符串

	#include <string.h>
	char *strdup(const char *s);
strdup()会先用maolloc()配置与参数s字符串相同的空间大小，然后将参数s字符串的内容复制到该内存地址，然后把该地址返回。该地址最后可以利用free()来释放。

返回一字符串指针，该指针指向复制后的新字符串地址。若返回NULL表示内存不足。
##toascii
将整型数转换成合法的ASCII 码字符

	#include <ctype.h>
	int toascii(int c);
toascii()会将参数c转换成7位的unsigned char值，第八位则会被清除，此字符即会被转成ASCII码字符。相当于与上0x7F。

将转换成功的ASCII码字符值返回。
##部分网络相关函数
```
#include <netinet/in.h>
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int inet_aton(const char *cp, struct in_addr *inp);
char *inet_ntoa(struct in_addr in);
unsigned long int inet_addr(const char *cp);
struct in_addr
{
	unsigned long int s_addr;
};
inet_aton()用来将参数cp所指的网络地址字符串转换成网络使用的二进制的数字，
然后存于参数inp所指的in_addr结构中。成功返回非0值，失败返回0。（与一般成功返回0不同）
inet_ntoa()用来将参数in所指的网络二进制的数字转换成网络地址，
然后将指向此网络地址字符串的指针返回，字符串存储于静态缓冲区。失败返回NULL。
inet_addr()用来将参数cp所指的网络地址字符串转换成网络所使用的二进制数字。
网络地址字符串是以数字和点组成的字符串，例如:“163.13.132.68”。成功则返回对应的网络二进制的数字，失败返回-1。

#include <sys/socket.h>
int shutdown(int s, int how);
shutdown()用来终止参数s所指定的socket连接。参数how有下列几种情况:
how=0 终止读取操作。
how=1 终止传送操作
how=2 终止读取及传送操作
成功则返回0，失败返回-1，错误原因存于errno。
```
***
本篇及其他相关篇目主要总结自：http://man.chinaunix.net/develop/c&c++/linux_c/default.htm