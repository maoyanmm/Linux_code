#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

void* thread_routine(void* arg)
{
    (void)arg;
    printf("工作线程在跑... ...\n");
    pthread_detach(pthread_self());
    int* p = (int*)malloc(sizeof(int));
    *p = 110;
    return (void*)p;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid,NULL,thread_routine,NULL);
    //sleep是为了保证工作线程是分离好了的，再执行主线程的逻辑
    sleep(3);
    void* ret;
    int tmp = pthread_join(tid,&ret);
    if(tmp == 0)
    {
        printf("等待成功！收到了工作线程的返回值：[%d]\n",*((int*)ret));
    }
    else
    {    
        printf("等待失败！\n");
    }

    return 0;
}
