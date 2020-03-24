#include<pthread.h>
#include<unistd.h>
#include<queue>
#include<stdio.h>
#include<iostream>

#define THREAD_COUNT 5

typedef void(*handler_t)(int);

void Function(int num)
{
    printf("num + 1 = %d\n",num+1);
}

class ThreadTask
{
private:
    int _data;
    handler_t _handler;
public:
    ThreadTask(int data = 0, handler_t hand = NULL)
    {
        _data = data;
        _handler = hand;
    }
    void Run()
    {
        _handler(_data);
    }
};

class ThreadPool
{
private:
   pthread_t _tid[THREAD_COUNT];
   std::queue<ThreadTask*> _task_queue;
   pthread_mutex_t _mtx;
   pthread_cond_t _thread_queue;
   int _thread_capacity;
   int _thread_size;
   bool _is_quit;
public:
   ThreadPool()
       :_thread_capacity(THREAD_COUNT)
        ,_thread_size(0)
        ,_is_quit(false)
   {
       pthread_cond_init(&_thread_queue,NULL);
       pthread_mutex_init(&_mtx,NULL);
       ThreadInit();
   }
   void ThreadInit()
   {
       for(int i = 0; i < THREAD_COUNT; ++i)
       {
           int ret = pthread_create(&_tid[i],NULL,ThreadStart,(void*)this);
           if(ret != 0)
           {
               perror("pthread_create");
           }
           else
           {
               ++_thread_size;
           }
       }
   }
   ~ThreadPool()
   {
        pthread_cond_destroy(&_thread_queue);
        pthread_mutex_destroy(&_mtx);
   }
   void Pop(ThreadTask** task)
   {
        *task = _task_queue.front();
        _task_queue.pop();
   }
   bool Push(ThreadTask* task)
   {
       pthread_mutex_lock(&_mtx);
       if(_is_quit)
       {
           pthread_mutex_unlock(&_mtx);
           return false;
       }
       _task_queue.push(task);
       pthread_mutex_unlock(&_mtx);
       pthread_cond_signal(&_thread_queue);
       return true;
   }
   void Quit()
   {
       pthread_mutex_lock(&_mtx);
       _is_quit = true;
       pthread_mutex_unlock(&_mtx);

       while(_thread_size > 0)
       {
           pthread_cond_broadcast(&_thread_queue);
       }
       printf("线程池已安全退出\n");
   }
private:
    static void* ThreadStart(void* arg)
    {
        ThreadPool* tp = (ThreadPool*)arg;
        while(1)
        {
            pthread_mutex_lock(&tp->_mtx);
            while(tp->_task_queue.empty())
            {
                if(tp->_is_quit)
                {
                    printf("线程[%p]即将退出\n",pthread_self());
                    sleep(1);
                    --tp->_thread_size;
                    pthread_mutex_unlock(&tp->_mtx);
                    pthread_exit(NULL);
                }
                pthread_cond_wait(&tp->_thread_queue,&tp->_mtx);
            }
            ThreadTask* task = new ThreadTask();
            tp->Pop(&task);
            pthread_mutex_unlock(&tp->_mtx);
            task->Run();
            delete task;
        }
        return NULL;
    }
};

int main()
{
    ThreadPool tp;
    for(int i = 0; i < 20; ++i)
    {
        ThreadTask* tt = new ThreadTask(i,Function);
        tp.Push(tt);
    }
    tp.Quit();
    return 0;
}
