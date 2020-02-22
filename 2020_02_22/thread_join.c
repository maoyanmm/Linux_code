#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

void* thread_routine1(void* arg)
{
    //表示不用这个参数
    (void)arg;
    printf("第一个线程在跑... ...\n");
    sleep(5);
    int* p = (int*)malloc(sizeof(int));
    *p = 110;
    return (void*)p;
}

void* thread_routine2(void* arg)
{
    (void)arg;
    printf("第二个线程在跑... ...\n");
    sleep(5);
    int* p = (int*)malloc(sizeof(int));
    *p = 7474;
    pthread_exit((void*)p);
    return NULL;
}

void* thread_routine3(void* arg)
{
    (void)arg;
    while(1)
    {
        sleep(1);
        printf("第三个线程在跑... ...\n");
    }
    return NULL;
}

int main()
{
    printf("主线程开始... ...\n");
    pthread_t tid;
    printf("开始创建第一个线程... ...\n");
    pthread_create(&tid,NULL,thread_routine1,NULL);
    //用来接收等待的线程的返回值
    void* ret;
    pthread_join(tid,&ret);
    printf("收到了第一个线程[%p]的返回值：[%d]\n",tid,*((int*)ret));
    free(ret);


    printf("开始创建第二个线程... ...\n");
    pthread_create(&tid,NULL,thread_routine2,NULL);
    pthread_join(tid,&ret);
    printf("收到了第二个线程[%p]的返回值：[%d]\n",tid,*((int*)ret));
    free(ret);

    
    printf("开始创建第三个线程... ...\n");
    pthread_create(&tid,NULL,thread_routine3,NULL);
    sleep(10);
    pthread_cancel(tid);
    pthread_join(tid,&ret);
    if(ret == PTHREAD_CANCELED)
    {
        printf("收到了第三个线程[%p]的返回值：PTHREAD_CANCELED\n",tid);
    }
    else
    {
        printf("收到了第三个线程的返回值：NULL");
    }

    return 0;
}
