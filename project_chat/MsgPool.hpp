#pragma once
#include<queue>
#include<pthread.h>
#include<iostream>
#include<string>

#define MSG_POOL_SIZE 1024

class MsgPool
{
    private:
        std::queue<std::string> _msg_queue;
        //队列大小
        size_t _capacity;
        pthread_mutex_t _mtx;
        //消费者条件变量
        pthread_cond_t _consume_cond;
        //生产者条件变量
        pthread_cond_t _product_cond;
    public:
        MsgPool()
        {
            _capacity = MSG_POOL_SIZE;
            pthread_mutex_init(&_mtx,NULL);
            pthread_cond_init(&_consume_cond,NULL);
            pthread_cond_init(&_product_cond,NULL);
        }
        ~MsgPool()
        {
            pthread_mutex_destroy(&_mtx);
            pthread_cond_destroy(&_consume_cond);
            pthread_cond_destroy(&_product_cond);
        }
        void PushMsgToPool(const std::string& msg)
        {
            pthread_mutex_unlock(&_mtx);
            while(IsFull())
            {
                pthread_cond_wait(&_product_cond,&_mtx);
            }
            _msg_queue.push(msg);    
            pthread_mutex_unlock(&_mtx);
            pthread_cond_signal(&_consume_cond);
        }
        void PopMsgFromPool(std::string* msg)
        {
            pthread_mutex_unlock(&_mtx);
            while(IsEmpty())
            {
                pthread_cond_wait(&_consume_cond,&_mtx);
            }
            *msg = _msg_queue.front();
            _msg_queue.pop();
            pthread_mutex_unlock(&_mtx);
            pthread_cond_signal(&_product_cond);
        }
    private:
        bool IsFull()
        {
            return (_msg_queue.size() == _capacity);
        }
        bool IsEmpty()
        {
            return _msg_queue.empty();
        }
};
