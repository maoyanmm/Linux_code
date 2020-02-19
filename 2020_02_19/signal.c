#include<stdio.h>
#include<stdlib.h>
#include<signal.h>

void handler(int sig)
{
    printf("catch a signal : [%d]\n",sig);
}

int main()
{
    signal(2,handler);
    while(1)
    {

    }
    return 0;
}
