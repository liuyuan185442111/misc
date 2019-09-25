# getopt
```c
#include <getopt.h> // man里说是#include <unistd.h>
int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
```
如果选项成功找到，返回选项字符；如果所有命令行选项都解析完毕，返回 -1；如果遇到选项字符不在 optstring 中，返回字符 '?'，并将 optopt 置为该选项字符；如果遇到丢失参数，返回 '?' 或 ':'，当返回返回 '?' 时且 opterr 非 0 会提示错误信息。

optarg —— 指向当前选项参数（如果有）的指针或NULL（无参数）。
optind —— 再次调用 getopt() 时下一个 argv-element 的索引。
optopt —— 最后一个未知选项。
opterr ­—— 是否希望 getopt() 向 stderr 打印出错信息，0 为不打印，opterr 默认非0。

optstring：由三部分组成，第一部分是可选的字符'+'或'-'，第二部分是一个可选的字符';'，第三部分是具体的选项字符串。
举例说明第三部分 “ab:c::d:e”，abcde 分别是选项字符，选项字符其后跟 ":" 表示该选项有参数，选项跟参数之间可有空格，也可没有空格，选项字符后跟 "::" 表示参数可有可无，但如果有参数选项和参数之间不能有空格，选项字符后不跟 ":" 或 "::" 表示该选项没参数。"-ae -b100 -c -d 200"就是一个对应于上述 optstring 的命令选项，没参数的选项可以写在一起，比如 -ae。
getopt() 默认情况下会改变参数的顺序，从而使 nonoption argv-element 移至所有选项之后，但如果 optstring 第一部分是字符 '+'，或者设置了环境变量POSIXLY_CORRECT，第一个 nonoption argv-element 出现时立即停止选项处理，例如 “+ab:c::d:e”，"-ae 100 -c -d 200" 中 100 及其以后的参数都不再作为选项处理。
如果第一部分是 '-'，每个 nonoption argv-element 作为选项 1 的参数，这里的 1 是数字 1 不是字符 1。
如果第二部分的 ':' 存在，当丢失选项参数时 getopt() 不再返回 '?' 而是返回 ':'，且不会打印出错信息。

The special argument "--" forces an end of option-scanning regardless of the scanning mode.
```c
#include <stdio.h>
#include <getopt.h>
int main(int argc, char *argv[])
{
        int opt;
        const char *optstring = "ab:c::d:e";
        while (((opt = getopt(argc, argv, optstring))) != -1)
        {
                printf("opt = %c(%d)\t", opt, opt);
                printf("optarg = %s\t", optarg);
                printf("optind = %d\t", optind);
                printf("argv[optind] = %s\t", argv[optind]);
                printf("optopt = %c\(%d)n", optopt, optopt);
        }
        printf("after processing optind = %d, argv[optind] = %s\n", optind, argv[optind]);
}

./a.out file -ae -b100 -c -z -d 200
opt = a(97)     optarg = (null) optind = 2      argv[optind] = -ae      optopt = (0)
opt = e(101)    optarg = (null) optind = 3      argv[optind] = -b100    optopt = (0)
opt = b(98)     optarg = 100    optind = 4      argv[optind] = -c       optopt = (0)
opt = c(99)     optarg = (null) optind = 5      argv[optind] = -z       optopt = (0)
./a.out: invalid option -- 'z'
opt = ?(63)     optarg = (null) optind = 6      argv[optind] = -d       optopt = z(122)
opt = d(100)    optarg = 200    optind = 8      argv[optind] = (null)   optopt = z(122)
after processing optind = 7, argv[optind] = file

./a.out file  -ae -- -b100 -c -z -d 200
opt = a(97)     optarg = (null) optind = 2      argv[optind] = -ae      optopt = (0)
opt = e(101)    optarg = (null) optind = 3      argv[optind] = --       optopt = (0)
after processing optind = 3, argv[optind] = file
```
# getopt_long
```c
#include <getopt.h>
struct option {
    const char *name;
    int         has_arg;
    int        *flag;
    int         val;
};
int getopt_long(int argc, char * const argv[], const char *optstring,
           const struct option *longopts, int *longindex);
int getopt_long_only(int argc, char * const argv[], const char *optstring,
           const struct option *longopts, int *longindex);
```
The getopt_long() function works like getopt() except that it also accepts long options, started with two dashes. A long option may take a parameter, of the form --arg=param or --arg param.

option 的成员意义如下：
name
长选项的名字。
has_arg
no_argument (or 0) 选项没有参数；required_argument (or 1) 选项有一个参数ment; optional_argument (or 2) 选项可有可无参数。
flag
标识结果如何返回。选项如果被找到，如果 flag 是 NULL，getopt_long() 将会返回 val，否则 getopt_long() 返回 0，而且 *flag 被设置为 val；选项如果未被找到，getopt_long() 返回 '?'，*flag（如果 flag 不为 NULL ） 保持不变。
val
标识返回值。

longopts 的最后一个元素必须 be filled with zeros。
如果 longindex 不是 NULL，*longindex 将被设置为长选项在 longopts 中的索引。

getopt_long_only() is like getopt_long(), but '-' as well as "--" can indicate a long option. If an option that starts with '-' (not "--") doesn't match a long option, but does match a short option, it is parsed as a short option instead.
# 参考
https://linux.die.net/man/3/getopt
