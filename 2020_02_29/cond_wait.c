#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#define PTHREAD_NUM 4
//注意这里用两个cond，为了让生产者和消费者有不同的PCB等待队列
pthread_cond_t g_eatCond;
pthread_cond_t g_makeCond;
pthread_mutex_t g_mutex;
int g_food = 0;

void* make_food(void* arg)
{
    while(1)
    {
        (void)arg;
        pthread_mutex_lock(&g_mutex);
        //这里必须用while去判断，否则会导致多eat的情况
        while(g_food == 0)
        {
            pthread_cond_wait(&g_eatCond,&g_mutex);    
        }
        printf("这里有[%d]个食物，我要吃一个\n",g_food);
        --g_food;
        //操作完了再解锁
        pthread_mutex_unlock(&g_mutex);
        //解锁完了再唤醒make的队列
        pthread_cond_broadcast(&g_makeCond);
    }
    return NULL;
}

void* eat_food(void* arg)
{
    (void)arg;
    while(1)
    {
        pthread_mutex_lock(&g_mutex);
        //如果是if，会导致多make的情况
        while(g_food == 1)
        {
            pthread_cond_wait(&g_makeCond,&g_mutex);
        }
        printf("这里有[%d]个食物，我要放一个\n",g_food);
        ++g_food;
        //++临界资源后再释放锁
        pthread_mutex_unlock(&g_mutex);
        //释放锁之后再唤醒eat队列
        pthread_cond_broadcast(&g_eatCond);
    }
    return NULL;
}


int main()
{
    pthread_cond_init(&g_makeCond,NULL); 
    pthread_cond_init(&g_eatCond,NULL);
    pthread_mutex_init(&g_mutex,NULL);
    pthread_t producer[PTHREAD_NUM];
    pthread_t customer[PTHREAD_NUM];
    for(int i = 0; i < PTHREAD_NUM; ++i)
    {
        int ret = pthread_create(&producer[i],NULL,make_food,NULL);
        if(ret != 0)
        {
            printf("producer phtread create failed!\n");
            return 0;
        }
        ret = pthread_create(&customer[i],NULL,eat_food,NULL);
        if(ret != 0)
        {
            printf("producer phtread create failed!\n");
            return 0;
        }
    }
    for(int i = 0; i < PTHREAD_NUM; ++i)
    {
        pthread_join(producer[i],NULL);
        pthread_join(customer[i],NULL);
    }
    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_eatCond);
    pthread_cond_destroy(&g_makeCond);
    return 0;
}
