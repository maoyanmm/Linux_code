#include<iostream>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<queue>
#define PTHREAD_NUM 4

class BlockQueue
{
public:
    BlockQueue(int cap = 1)
        :_capacity(cap)
    {
        pthread_mutex_init(&_mutex,NULL);
        pthread_cond_init(&_condProd,NULL);
        pthread_cond_init(&_condCons,NULL);
    }
    ~BlockQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_condCons);
        pthread_cond_destroy(&_condProd);
    }
    bool Is_Full()
    {
        int size = _bq.size();
        if(size == _capacity)
        {
            return true;
        }
        return false;
    }
    bool Is_Empty()
    {
        int size = _bq.size();
        if(size == 0)
        {
            return true;
        }
        return false;
    }
    void Pop_Data(int& data)
    {
       pthread_mutex_lock(&_mutex);
       while(Is_Empty())
       {
           //进来while就说明是空的了，就要去通知生产者生产了
           pthread_cond_signal(&_condProd);
           pthread_cond_wait(&_condCons,&_mutex);
       }
       data = _bq.front();
       _bq.pop();
       pthread_mutex_unlock(&_mutex);
       //做完了也得提醒
       pthread_cond_signal(&_condProd);
    }
    void Push_Data(const int& data)
    {
        pthread_mutex_lock(&_mutex);
        while(Is_Full())
        {
            pthread_cond_signal(&_condCons);
            pthread_cond_wait(&_condProd,&_mutex);
        }
        _bq.push(data);
        pthread_mutex_unlock(&_mutex);
        pthread_cond_signal(&_condCons);
    }

private:
    std::queue<int> _bq;
    int _capacity;
    pthread_mutex_t _mutex;
    //消费者的条件变量
    pthread_cond_t _condCons;
    //生产者的条件变量
    pthread_cond_t _condProd;
};

void* Produce(void* arg)
{
    BlockQueue* bq = (BlockQueue*)arg;
    int makeData = 0;
    while(1)
    {
        bq->Push_Data(makeData);
        //注意这里不是原子性的
        //可能刚刚push完还没来得及打印，时间片就到了
        //所以打印的时候可能错乱
        printf("我[%p]做了一个[%d]\n",pthread_self(),makeData);
        ++makeData;
    }
    return NULL;
}
void* Consume(void* arg)
{
    BlockQueue* bq = (BlockQueue*)arg;
    int getData = 0;
    while(1)
    {
        bq->Pop_Data(getData);
        //注意这里不是原子性的
        //可能刚刚pop完还没来得及打印，时间片就到了
        //所以打印的时候可能错乱
        printf("我[%p]吃了一个[%d]\n",pthread_self(),getData);
    }
    return NULL;
}

int main()
{
    //注意这里是在主函数创建对象，而不是在两个线程里创建
    //在所有的生产者和消费者线程的传参的时候，把同一个bq穿进去
    //为了所有线程使用同一个锁和分类的条件变量
    BlockQueue bq(2);
    pthread_t producer[PTHREAD_NUM];
    pthread_t consumer[PTHREAD_NUM];
    int i = 0;
    for(; i < PTHREAD_NUM; ++i)
    {
        int ret = pthread_create(&producer[i],NULL,Produce,(void*)&bq);
        if(ret != 0)
        {
            printf("创建线程producer[%d]失败！\n",i);
            return 0;
        }
        ret = pthread_create(&consumer[i],NULL,Consume,(void*)&bq);
        if(ret != 0)
        {
            printf("创建线程consumer[%d]失败！\n",i);
            return 0;
        }
    }
    for(int i = 0; i < PTHREAD_NUM; ++i)
    {
        pthread_join(consumer[i],NULL);
        pthread_join(producer[i],NULL);
    }
    return 0;
}
