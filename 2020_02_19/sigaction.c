#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<unistd.h>
//默认的结构体
struct sigaction old_act;
void sigcallback(int sig)
{
    printf("获取到信号号码：[%d]\n",sig);
    sigaction(sig,&old_act,NULL);
}
int main()
{
    //包含自定义信号处理函数的结构体
    struct sigaction new_act;
    //将自定义的结构体里的调用的函数指向自己定义的信号处理函数
    new_act.sa_handler = sigcallback;
    //置为0是为了让这个结构体默认调用的是sa_handler函数
    new_act.sa_flags = 0;
    //将等待的位图置0
    sigemptyset(&new_act.sa_mask);
    
    sigaction(SIGINT,&new_act,&old_act);
    while(1)
    {
        printf("执行中\n");
        sleep(1);
    }

    return 0;
}
