#include<iostream>
#include<stdio.h>
#include<semaphore.h>
#include<vector>
#include<pthread.h>
#define RESOURCE 1
#define PTHREAD_NUM 4 

template<class T>
class CircularQueue
{
public:
    CircularQueue(size_t capacity = 0)
        :_capacity(capacity)
         ,_read(0)
         ,_write(0)
    {
        _vc.resize(_capacity);
        sem_init(&_consumer,0,0);
        sem_init(&_producer,0,RESOURCE);
        sem_init(&_lock,0,1);
    }
    ~CircularQueue()
    {
        sem_destroy(&_consumer);
        sem_destroy(&_producer);
        sem_destroy(&_lock);
    }
    void Pop(T* data)
    {
        //一定是先P操作是消费者，再是P操作锁
        //因为可能进了锁之后会发生没有消费资源的情况，而导致锁无法释放
        sem_wait(&_consumer);
        sem_wait(&_lock);
        //读出数据、更新read读的下标
        *data = _vc[_read];
        //把他放在里面打印是因为：
        //如果放在外面打印可能会刚消费了数据，这个线程就被挂起，打印要等到得到CPU才打印
        printf("线程[%p]消费了数据[%d]\n",pthread_self(),*data);
        _read = (_read + 1) % _capacity; 
        //两个post顺序不会影响安全
        sem_post(&_lock);
        sem_post(&_producer);
    }
    void Push(const T& data)
    {
        //这里也必须是P操作生产者，再P操作锁
        //因为否则可能会导致进了锁之后会发生没有生产者资源的情况，从而导致锁无法V(释放)
        sem_wait(&_producer);
        sem_wait(&_lock);
        //写入数据，更新write写的下标
        _vc[_write] = data;
        //如果放在外面打印可能会刚消费了数据，这个线程就被挂起，打印要等到得到CPU才打印
        printf("线程[%p]生产了数据[%d]\n",pthread_self(),data);
        _write = (_write + 1) % _capacity;
        //两个post顺序不会影响安全
        sem_post(&_lock);
        sem_post(&_consumer);
    }

private:
    sem_t _consumer;
    sem_t _producer;
    sem_t _lock;
    //环形队列：由vector实现
    std::vector<T> _vc;
    size_t _capacity;
    //读写的下标
    size_t _read;
    size_t _write;
};

void* Consume(void* arg)
{
    CircularQueue<int>* cq = (CircularQueue<int>*)arg;
    int readData = 0;
    while(1)
    {
        cq->Pop(&readData);
    }
    return NULL;
}

void* Produce(void* arg)
{
    CircularQueue<int>* cq = (CircularQueue<int>*)arg;
    int writeData = 0;
    while(1)
    {
        cq->Push(writeData);
        ++writeData;
    }
    return NULL;
}

int main()
{
    CircularQueue<int>* cq = new CircularQueue<int>(4);
    pthread_t cons[PTHREAD_NUM];
    pthread_t prod[PTHREAD_NUM];
    for(int i = 0; i < PTHREAD_NUM; ++i)
    {
        int ret = pthread_create(&cons[i],NULL,Consume,(void*)cq);
        if(ret != 0)
        {
            std::cout <<"Pthread create failed!" << std::endl;
        }
        ret = pthread_create(&prod[i],NULL,Produce,(void*)cq);
    }
    for(int i = 0; i < PTHREAD_NUM; ++i)
    {
        pthread_join(cons[i],NULL);
        pthread_join(prod[i],NULL);
    }
    delete cq;
    cq = NULL;
    return 0;
}
