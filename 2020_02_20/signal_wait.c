#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>

void sigcallback(int sig)
{
    printf("收到了信号号码[%d]\n",sig);
    //默认处理SIGCHLD信号是忽略的
    //这里直接调用自定义的处理函数，然后在这里去waitSIGCHLD
    wait(NULL);
}

int main()
{
    signal(2,sigcallback);
    signal(SIGCHLD,sigcallback);
    pid_t pid = fork();
    if(pid < 0)
    {
        perror("fork");
        return 0;
    }
    else if(pid == 0)
    {
        printf("i am child! i want to exit!\n");
        exit(1);
    }
    else
    {
        printf("i am father\n");
        while(1)
        {
            sleep(2);
            printf("我不接受子进程的退出信号\n");
        }
    }
    return 0;
}
