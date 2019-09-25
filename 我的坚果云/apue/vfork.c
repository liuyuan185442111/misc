#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//8.4节 vfork函数
//vfork用于创建一个新进程, 而该新进程的目的是exec一个新程序, 它不将父进程的地址空间完全复制到子进程中,
//在子进程调用exec或exit之前, 它在父进程的空间中运行.
//vfork保证子进程先运行, 在子进程调用exec或exit之后父进程才可能被调度运行.
int glob = 6;
int main()
{
    int var=88;
    pid_t pid;
    puts("before vfork");
    if((pid=vfork())<0)
    {
        exit (0);
    }
    else if(pid == 0)
    {
        glob++;
        var++;
		//修改的是父进程中的glob和var
        printf("in child:pid=%d,glob=%d,var=%d\n",getpid(),glob,var);
        //fclose(stdout);
        _exit(0);
    }
    char buf[20];
    int i = printf("in parent:pid=%d,glob=%d,var=%d\n",getpid(),glob,var);
    sprintf(buf,"print %d chars\n",i);
    write(STDOUT_FILENO,buf,strlen(buf));
    return 0;
}
//如果子进程在exit前关闭stdout, 那父进程终的printf会失败, 但STDOUT_FILENO仍有效, 子进程得到的是父进程的文件描述符数组的副本.
