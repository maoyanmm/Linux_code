//为了：拿数据和取数据
#include<string>
#include<iostream>
#include<queue>
#include<pthread.h>

//数据池可以存放数据的个数
#define DATA_POOL_SIZE 1024

class DataPool
{
private:
    //消息队列
    std::queue<std::string> _data_queue;
    //防止队列无限扩容
    size_t _capacity;
    //消费者条件变量
    pthread_cond_t _consume_cond;
    //生产者条件变量
    pthread_cond_t _produce_cond;
    pthread_mutex_t _mutex;
public:
    DataPool()
    {
        _capacity = DATA_POOL_SIZE;
        pthread_cond_init(&_consume_cond,NULL);
        pthread_cond_init(&_produce_cond,NULL);
        pthread_mutex_init(&_mutex,NULL);
    }
    ~DataPool()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_consume_cond);
        pthread_cond_destroy(&_produce_cond);
    }
    void PushDataToPool(std::string& data)
    {
        pthread_mutex_lock(&_mutex);
        while(IsFull())
        {
            //进入等待队列前先去催消费
            pthread_cond_signal(&_consume_cond);
            pthread_cond_wait(&_produce_cond,&_mutex);
        }
        _data_queue.push(data);
        pthread_mutex_unlock(&_mutex);
        pthread_cond_signal(&_consume_cond);
    }
    void PopDataFromPool(std::string& data)
    {
        pthread_mutex_lock(&_mutex);
        while(IsEmpty())
        {
            //进入等待队列前先去催生产
            pthread_cond_signal(&_produce_cond);
            pthread_cond_wait(&_consume_cond,&_mutex);
        }
        data = _data_queue.front();
        _data_queue.pop();
        pthread_mutex_unlock(&_mutex);
        pthread_cond_signal(&_produce_cond);
    }
private:
    bool IsEmpty()
    {
        return _data_queue.size() == 0;
    }
    bool IsFull()
    {
        return _data_queue.size() == _capacity;
    }
};
