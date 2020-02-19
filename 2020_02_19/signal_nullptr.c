#include<stdlib.h>
#include<signal.h>
#include<stdio.h>
void handler(int sig)
{
    printf("获取到异常的信号号码：[%d]\n",sig);
}

int main()
{
    signal(SIGSEGV,handler);
    int* p = NULL;
    *p = 10;
    return 0;
}
