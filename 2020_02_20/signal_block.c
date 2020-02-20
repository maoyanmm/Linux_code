#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>

void sigcallback(int sig)
{
    printf("收到了信号号码：[%d]\n",sig);
}

int main()
{
    signal(2,sigcallback);
    signal(40,sigcallback);
    sigset_t set;
    sigset_t oldset;
    //将原来的位置为全0
    sigemptyset(&oldset);
    //将现在的位置为全1
    sigfillset(&set);
    //将所有的信号设置为阻塞状态，这里是不包含9和19信号的
    sigprocmask(SIG_BLOCK,&set,&oldset);
    printf("test sigal... ...\n");
    sleep(20);
    //sleep完之后会将所有的信号设置为非阻塞状态
    sigprocmask(SIG_UNBLOCK,&set,&oldset);
    while(1)
    {
        printf("test sigal... ...\n");
        sleep(2);
    }
    return 0;
}
