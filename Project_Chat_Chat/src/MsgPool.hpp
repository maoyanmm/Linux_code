#pragma once
#include<pthread.h>
#include<queue>
#include<string>

//数据池模块就是给服务端收发信息做缓冲用的

#define MSG_POOL_SIZE 1024

class MsgPool
{
    private:
        //保证了生产者和消费者的放和取的安全
        pthread_mutex_t _lock;
        //存放数据的队列
        std::queue<std::string> _msg_queue;
        //数据池的大小
        size_t _capacity;
        //消费者条件变量
        pthread_cond_t _consume_cond;
        //生产者条件变量
        pthread_cond_t _product_cond;
    public:
        MsgPool()
        {
            pthread_mutex_init(&_lock,NULL);
            pthread_cond_init(&_consume_cond,NULL);
            pthread_cond_init(&_product_cond,NULL);
            _capacity = MSG_POOL_SIZE;
        }
        ~MsgPool()
        {
            pthread_mutex_destroy(&_lock);
            pthread_cond_destroy(&_consume_cond);
            pthread_cond_destroy(&_product_cond);
        }
        void PushMsgToPool(const std::string& msg)
        {
            pthread_mutex_lock(&_lock);
            while(IsFull())
            {
                pthread_cond_signal(&_consume_cond);
                pthread_cond_wait(&_product_cond,&_lock);
            }
            _msg_queue.push(msg);
            pthread_mutex_unlock(&_lock);
            pthread_cond_signal(&_consume_cond);
        }
        void PopMsgFromPool(std::string* msg)
        {
            pthread_mutex_lock(&_lock);
            while(IsEmpty())
            {
                pthread_cond_signal(&_product_cond);
                pthread_cond_wait(&_consume_cond,&_lock);
            }
            *msg = _msg_queue.front();
            _msg_queue.pop();
            pthread_mutex_unlock(&_lock);
            pthread_cond_signal(&_product_cond);
        }
    private:
        bool IsFull()
        {
            return _msg_queue.size() == _capacity;
        }
        bool IsEmpty()
        {
            return _msg_queue.empty();
        }
};
