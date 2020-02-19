#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
int main()
{
    //定时停止一个进程
    alarm(1);
    int count = 0;
    while(1)
    {
        ++count;

    printf("count = [%d]\n",count);
    }
    return 0;
}
