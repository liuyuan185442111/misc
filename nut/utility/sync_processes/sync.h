#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

//使用SIGUSR1和SIGUSR2实现父子进程同步
bool TELL_WAIT2();
void TELL_PARENT(pid_t pid);
void WAIT_PARENT();
void TELL_CHILD(pid_t pid);
void WAIT_CHILD();

//只使用SIGUSR1也行
bool TELL_WAIT1();
void TELL(pid_t pid);
void WAIT();
