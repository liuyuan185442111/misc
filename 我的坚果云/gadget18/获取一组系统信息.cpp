/* 输出一组系统信息
$ uname -a
Linux centos 2.6.32-642.6.2.el6.x86_64 #1 SMP Wed Oct 26 06:52:09 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
$ cat /etc/issue
CentOS release 6.8 (Final)
Kernel \r on an \m
*/
#include <stdio.h>
int is_little()
{
    union
    {
        int a;
        char b;
    } c = {1};
    return c.b == 1;
}
int main()
{
    puts  ("sizeof: int,long,long long,float,double,long double");
    printf("        %d,  %d,   %d,     ",sizeof(int),sizeof(long),sizeof(long long));
    printf("   %d,    %d,     %d\n",sizeof(float),sizeof(double),sizeof(long double));
    if(is_little()) puts("little endian");
    else puts("big endian");
#ifdef __GNUC__
    puts("__GNUC__");
#endif
#ifdef __x86_64__
    puts("64 bits");
#endif
#ifdef __i386__
    puts("32 bits");
#endif
    return 0;
}
/* 某个系统上的运行结果
sizeof: int,long,long long,float,double,long double
        4,  8,   8,        4,    8,     16
little endian
__GNUC__
64 bits
*/
