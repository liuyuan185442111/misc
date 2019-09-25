历史上大多数UNIX系统支持main函数带有三个参数：

	int main(int argc, char *argv[], char *env[]);
其中第二个参数是参数表的地址，第三个参数就是环境表的地址，它们都是一个字符指针数组，每个指针指向一个以null结束的C字符串。
另外全局变量environ也包含了环境表指针数组的地址：
	
	extern char **environ;
因为ISO C规定main函数只有两个参数，所以POSIX.1也规定应使用environ而不使用第三个参数。
下面的例子展示了main函数的参数：
```
include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <assert.h>

extern char **environ;

int main(int argc, char *argv[], char *env[])
{
    printf("sizeof pointer: %d\n", sizeof(void*));
	assert(argc == *(int*)(argv-1));
    printf("Argument count: %d\n", argc);
    int i = 0;
    for(; i<argc; ++i)
    {
        printf("Argument %d: %s\n", i, argv[i]);
    }
    assert(argv[argc] == NULL);
	char **p = argv+argc+1;
    printf("Environment address: %p %p %p\n", p, env, environ);
    printf("Environment:\n");
    while(*p)
    {
        printf("    %s\n", *p);
        p++;
    }
    assert(*p++ == NULL);
    printf("Auxiliary vectors:\n");
#ifdef __x86_64__
    Elf64_auxv_t *aux = (Elf64_auxv_t*)p;
#else
    Elf32_auxv_t *aux = (Elf32_auxv_t*)p;
#endif
    while(aux->a_type != AT_NULL)
    {
        printf("    Type: %2d Value: %x\n", aux->a_type, aux->a_un.a_val);
		++aux;
    }
    return 0;
}

```
C程序的典型存储器安排是这样的：

	命令行参数和环境变量   高地址
	------------------
	栈
	------------------
	堆
	------------------
	未初始化的数据
	------------------
	初始化的数据
	------------------
	程序正文              低地址
环境表和环境字符串通常占用的是进程地址空间顶部，它下面是栈空间，所以它不能向上(高地址)又不能向下(低地址)扩展。如果要增加一个环境变量，操作就比较复杂了。
如果是第一次增加一个新name，调用malloc为新的环境表分配空间，然后将原来的环境表复制过来，并将指向新name=value字符串的指针放到环境表的尾部，再将一个空指针放在其后，最后更新environ使之指向新环境表。由于evnp是值传递的，它的值并不会进行更新。
如果不是第一次增加一个新name，只要调用realloc扩展空间，然后将指向新name=value字符串的指针放到环境表的尾部，再将一个空指针放在其后。此时environ的值可能变化也可能不变。
第一次增加新name时，复制原来的环境表这一步可能是通过environ去寻找旧的环境表，所以用户程序最好不要修改environ的值。
有两个函数可以增加一个新的环境变量：

	#include <stdlib.h>
	int putenv(chat *str);
	int setenv(const char *name, const char *value, int rewrite);
setenv会分配存储区，以便存放name=value字符串。putenv则可能直接把传递给它的字符串地址放入环境表尾部，如果这字符串是存放在栈中则可能会发生错误。
以下例程展示了增加一个环境变量的操作：
```
#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(int argc, char *argv[])
{
    printf("environ is %p\n",environ);
	char **p = environ;
	while(*p) ++p;
    if(environ) printf("last element address is %p\n",p[-1]);
    char buf[64]={"name=value"};
    printf("putenv\nbuf address is p\n",buf);
    putenv(buf);
    printf("new environ is %p\n",environ);
	p = environ;
	while(*p) ++p;
    if(environ)
    {
        printf("the one before last address is %p\n",p[-2]);
        printf("last element address is %p\n",p[-1]);
    }
    return 0;
}
```
参考
程序员的自我修养
UNIX环境高级编程 7.6 C程序的存储空间布局
UNIX环境高级编程 7.6 环境变量