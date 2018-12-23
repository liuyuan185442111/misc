#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <sys/shm.h>

bool TELL_WAIT()
{
    if(signal(SIGUSR1, sig_usr) != 0) return false;
    if(sigemptyset(&zeromask) != 0) return false;
    if(sigemptyset(&newmask) != 0) return false;
    if(sigaddset(&newmask, SIGUSR1) != 0) return false;
    if(sigprocmask(SIG_BLOCK, &newmask, &oldmask) != 0) return false;
    return true;
}
void TELL(pid_t pid)
{
    kill(pid, SIGUSR1);
}
void WAIT()
{
    while(sigflag == 0) sigsuspend(&zeromask);
    sigflag = 0;
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

static int update(int *ptr)
{
	return ((*ptr)++);
}

int main()
{
        int shmid = shmget(IPC_PRIVATE, sizeof(int), 0600);
        assert(shmid != -1);

        TELL_WAIT2();

        pid_t pid = fork();
        if(pid < 0) return 3;
        else if(pid > 0)
        {
                void *area = shmat(shmid, 0, 0);
                assert(area != (void*)-1);
                printf("parent shm addr: %p\n", area);
                int tmep;
                for(int i=0; i<100; i+=2)
                {
                        printf("parent %d\n",update((int*)area));
                        //TELL(pid);
                        //WAIT();
                        TELL_CHILD(pid);
                        WAIT_CHILD();
                }
                shmdt(area);
        }
        else
        {
                void *area = shmat(shmid, 0, 0);
                assert(area != (void*)-1);
                printf("child shm addr: %p\n", area);
                for(int i=1; i<101; i+=2)
                {
                        WAIT_PARENT();
                        //WAIT();
                        printf("child %d\n",update((int*)area));
                        TELL_PARENT(getppid());
                        //TELL(getppid());
                }
                shmdt(area);
                shmctl(shmid, IPC_RMID, 0);
        }
        return 0;
}