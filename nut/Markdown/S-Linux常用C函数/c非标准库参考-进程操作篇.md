##_exit（结束进程执行）
    #include <unistd.h>
    void _exit(int status);
_exit()用来立刻结束目前进程的执行，并把参数status返回给父进程，并关闭未关闭的文件。此函数调用后不会返回，并且会传递SIGCHLD信号给父进程，父进程可以由wait函数取得子进程结束状态。
_exit()不会处理标准I/O缓冲区，如要更新缓冲区请使用exit()。
##on_exit（设置程序正常结束前调用的函数）
    #include <stdlib.h>
    int on_exit(void (* function)(int, void*), void *arg);
on_exit()用来设置一个程序正常结束前调用的函数。当程序通过调用exit()或从main中返回时，参数function所指定的函数会先被调用，然后才真正由exit()结束程序。参数arg会作为第二个参数传给function函数。

如果执行成功则返回0，否则返回-1，失败原因存于errno中。
```
#include <stdlib.h>
void my_exit(int status, void *arg)
{
    printf("before exit()!\n");
    printf("exit(%d)\n", status);
    printf("arg = %s\n", (char*)arg);
}
int main()
{
    char *str="test";
    on_exit(my_exit, (void *)str);
    exit(1234);
}

before exit()!
exit (1234)
arg = test
```
##参照标准库中相应的printf函数
    int vfscanf(FILE *stream, const char *format, va_list ap);
    int vscanf(const char *format, va_list ap);
    int vsprintf(char *str, const char *format, va_list ap);
##getpgid（取得进程组识别码）
    #include <unistd.h>
    pid_t getpgid(pid_t pid);
getpgid()用来取得参数pid指定进程所属的组识别码。如果参数pid为0，则会取得目前进程的组识别码。

执行成功则返回组识别码，如果有错误则返回-1，错误原因存于errno中。
ESRCH：找不到符合参数pid指定的进程。
##getpgrp（取得进程组识别码）
    #include <unistd.h>
    pid_t getpgrp(void);
getpgrp()用来取得当前进程所属的组识别码。此函数相当于调用getpgid(0);
##getpid（取得进程识别码）
    #include <unistd.h>
    pid_t getpid(void);
getpid()用来取得当前进程的进程识别码，许多程序利用取到的此值来建立临时文件，以避免临时文件名相同的问题。
##getppid（取得父进程的进程识别码）
    #include <unistd.h>
    pid_t getppid(void);
getppid()用来取得目前进程的父进程识别码。
##setpgid（设置进程组识别码）
    #include <unistd.h>
    int setpgid(pid_t pid, pid_t pgid);
setpgid()将参数pid指定进程所属的组识别码设为参数pgid指定的组识别码。如果参数pid为0，则会用来设置当前进程的组识别码，如果参数pgid为0，则会以当前进程的进程识别码来取代。一个进程只能为它自己或它的子进程设置进程组id。

执行成功则返回组识别码，如果有错误则返回-1，错误原因存于errno中。
EINVAL：参数pgid小于0。
EPERM：进程权限不足，无法完成调用。
ESRCH：找不到符合参数pid指定的进程。
##setpgrp（设置进程组识别码）
    #include <unistd.h>
    int setpgrp(void);
setpgrp()将当前进程所属的组识别码设为当前进程的进程识别码。此函数相当于调用setpgid(0,0)。

执行成功则返回组识别码，如果有错误则返回-1，错误原因存于errno中。
##nice（改变进程优先顺序）
    #include <unistd.h>
    int nice(int inc);
nice()用来改变进程的进程执行优先顺序。参数inc数值越大则优先顺序排在越后面，即表示进程执行会越慢。只有超级用户才能使用负的inc值，代表优先顺序排在前面，进程执行会较快。

如果执行成功则返回新的nice值，否则返回-1，失败原因存于errno中。

EPERM：一般用户企图使用负的参数inc值改变进程优先顺序。
##getpriority（取得程序进程执行优先权）
    #include <sys/time.h>
    #include <sys/resource.h>
    int getpriority(int which, int who);
getpriority()可用来取得进程、进程组和用户的进程执行优先权。which有三种数值，参数who则依which值有不同定义：
which who代表的意义
PRIO_PROCESS who为进程识别码
PRIO_PGRP who为进程的组识别码
PRIO_USER who为用户识别码
此函数返回的数值介于-20至20之间，代表进程执行优先权，数值越低代表有较高的优先次序，执行会较频繁。

返回进程执行优先权，如有错误发生返回值则为-1，且错误原因存于errno。
ESRCH：参数which或who可能有错，而找不到符合的进程。
EINVAL：参数which值错误。
##setpriority（设置程序进程执行优先权）
    #include <sys/time.h>
    #include <sys/resource.h>
    int setpriority(int which, int who, int prio);
setpriority()可用来设置进程、进程组和用户的进程执行优先权。参数which有三种数值，参数who则依which值有不同定义：
which who代表的意义
PRIO_PROCESS who为进程识别码
PRIO_PGRP who为进程的组识别码
PRIO_USER who为用户识别码
参数prio介于-20至20之间。优先权默认是0，只有超级用户（root）允许降低此值。

执行成功则返回0，如果有错误发生返回值则为-1，错误原因存于errno。
ESRCH：参数which或who可能有错，而找不到符合的进程。
EINVAL：参数which值错误。
EPERM：权限不够，无法完成设置。
EACCES：一般用户无法降低优先权。
##wait（等待子进程中断或结束）
    #include <sys/types.h>
    #include <sys/wait.h>
    pid_t wait(int *status);
wait()会暂时停止目前进程的执行，如果在调用wait()时子进程已经结束，则wait()会立即返回子进程结束状态值。子进程的结束状态值会由参数status返回，而子进程的进程识别码也会返回。如果不在意结束状态值，则参数status可以设成NULL。

如果执行成功则返回子进程识别码(PID)，如果有错误发生则返回-1。失败原因存于errno中。
```
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main()
{
        pid_t pid;
        int status;
        if(fork() == 0){
                sleep(1);
                printf("This is the child process. pid = %d\n", getpid());
                return 9;
        }else{
                printf("This is the parent process, wait for child...\n");
                pid = wait(&status);
                printf("child's pid = %d. exit status is %d\n", pid, WEXITSTATUS(status));
        }
        return 0;
}
```
##waitpid（等待子进程中断或结束）
    #include <sys/types.h>
    #include <sys/wait.h>
    pid_t waitpid(pid_t pid, int *status, int options);
参数pid为欲等待的子进程识别码，其他数值意义如下:
pid<-1 等待进程组识别码为pid绝对值的任何子进程。
pid=-1 等待任何子进程，相当于wait()。
pid=0 等待进程组识别码与目前进程相同的任何子进程。
pid>0 等待任何子进程识别码为pid的子进程。
参数option可以为0或下面的OR组合：
WNOHANG 如果没有任何已经结束的子进程则马上返回，不予以等待。
WUNTRACED 如果子进程进入暂停执行情况则马上返回，但结束状态不予以理会。
子进程的结束状态返回后存于status，底下有几个宏可判别结束情况：

    WIFEXITED(status) 如果子进程正常结束则为非0值
    WEXITSTATUS(status) 取得子进程exit()返回的结束代码，一般会先用WIFEXITED来判断是否正常结束才能使用此宏
    WIFSIGNALED(status) 如果子进程是因为信号而结束则此宏值为真
    WTERMSIG(status) 取得子进程因信号而中止的信号代码，一般会先用WIFSIGNALED来判断后才使用此宏
    WIFSTOPPED(status) 如果子进程处于暂停执行情况则此宏值为真。一般只有使用WUNTRACED 时才会有此情况
    WSTOPSIG(status) 取得引发子进程暂停的信号代码，一般会先用WIFSTOPPED来判断后才使用此宏

如果执行成功则返回子进程识别码(PID)，如果有错误发生则返回-1。失败原因存于errno中。
##system（执行shell命令）
    #include <stdlib.h>
    int system(const char *string);
system()会调用fork()产生子进程，由子进程来调用/bin/sh -c string来执行参数string字符串所代表的命令，此命令执行完后随即返回原调用的进程。在调用system()期间SIGCHLD信号会被暂时搁置，SIGINT和SIGQUIT信号则会被忽略。

如果system()在调用/bin/sh时失败则返回127，其他失败原因返回-1。若参数string为空指针(NULL)，则返回非零值。如果system()调用成功则最后会返回执行shell命令后的返回值，但是此返回值也有可能为system()调用/bin/sh失败所返回的127，因此最好能再检查errno来确认执行成功。
##exec和vfork
vfork与fork的区别：
1.vfork保证子进程先运行，在它调用exec或exit之后父进程才可能被调度运行。
2.fork要拷贝父进程的进程环境；vfork不需要完全拷贝父进程的进程环境，在子进程没有调用exec或exit之前，子进程与父进程共享进程环境，相当于线程的概念，此时父进程阻塞等待。

一般来说，使用vfork就是为了进一步的调用exec。当进程调用exec时，该进程完全由新程序替换，前后的进程id并未改变，exec只是用另一个新程序完全替换了当前进程的正文，数据，堆栈。
##vfork（建立一个新的进程）
    #include <unistd.h>
    pid_t vfork(void);
vfork()会产生一个新的子进程，只有当其中一进程试图修改欲复制的空间时才会做真正的复制动作。

如果vfork()成功则在父进程会返回新建立的子进程代码(PID)，而在新建立的子进程中则返回0。如果vfork失败则直接返回-1，失败原因存于errno中。
ENOMEM：内存不足，无法配置核心所需的数据结构空间。
##exec
    #include <unistd.h>
    int execl(const char *path, const char *arg, ...);
	int execlp(const char *file, const char *arg, ...);
	int execv (const char *path, char *const argv[]);
	int execvp(const char *file ,char *const argv[]);
	int execve(const char *path, char *const argv[], char *const envp[]);
	
path表示绝对文件路径，file表示文件名，exec会在PATH环境变量所指的目录中查找符合参数file的文件。exec找到文件后会执行该文件，参数以arg或argv传递给main函数。前两个版本中的arg及后续参数是传递给新程序main函数的argv参数，后三个版本中的argv参数是传递给新程序main函数的argv参数。前两个版本中，通过将最后一个参数设置为NULL来标识结束，后三个版本中通过将argv的最后一个元素设置为NULL来标识结束。envp是传递给执行文件的新环境变量数组。

如果执行成功则函数不会返回，执行失败则直接返回-1，失败原因存于errno中。
EACCES
1.欲执行的文件不具有用户可执行的权限。
2.欲执行的文件所属的文件系统以noexec方式挂上。
3.欲执行的文件或script翻译器非一般文件。
EPERM
1.进程处于被追踪模式，执行者并不具有root权限，欲执行的文件具有SUID或SGID位。
2.欲执行的文件所属的文件系统是以nosuid方式挂上，欲执行的文件具有SUID或SGID位，但执行者并不具有root权限。
E2BIG 参数数组过大
ENOEXEC 无法判断欲执行文件的执行文件格式，有可能是格式错误或无法在此平台执行。
EFAULT 参数path所指的字符串地址超出可存取空间范围。
ENAMETOOLONG 参数path所指的字符串太长。
ENOENT 参数path字符串所指定的文件不存在。
ENOMEM 核心内存不足。
ENOTDIR 参数path字符串所包含的目录路径并非有效目录。
EACCES 参数path字符串所包含的目录路径无法存取，权限不足
ELOOP 过多的符号连接。
ETXTBUSY 欲执行的文件已被其他进程打开而且正把数据写入该文件中。
EIO I/O存取错误。
ENFILE 已达到系统所允许的打开文件总数。
EMFILE 已达到系统所允许单一进程所能打开的文件总数。
EINVAL 欲执行文件的ELF执行格式不只一个PT_INTERP节区。
EISDIR ELF翻译器为一目录。
ELIBBAD ELF翻译器有问题。