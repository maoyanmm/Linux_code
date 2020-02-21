#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct Thread_info
{
    int _threadNum;
    Thread_info()
    {
        _threadNum = -1;
    }
}Thread_info;

void* thread_start(void* arg)
{
    Thread_info* info = (Thread_info*)arg;
    while(1)
    {
        printf("这是线程[%d]\n",info->_threadNum);
        sleep(2);
    }
    delete info;
}

int main()
{
    pthread_t tid;
    //char* p = "hello shishuai";
    int* num = new int;
    Thread_info* info = new Thread_info;
    for(int i = 0; i < 4; ++i)
    {
        info->_threadNum = i;
        pthread_create(&tid,NULL,thread_start,(void*)info);
    }
    while(1)
    {
        printf("这是主线程\n");
        sleep(2);
    }
    return 0;
}
