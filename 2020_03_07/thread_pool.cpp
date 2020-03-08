#include<iostream>
#include<pthread.h>
#include<stdio.h>
#include<queue>
#include<unistd.h>
#define PTHREADNUM 4

void Funtion1(int data)
{
    printf("处理数据[%d]\n",data);
}

typedef void (*handler_t)(int);
class PthreadTask
{
private:
    handler_t _handler;
    int _data; 
public:
    PthreadTask(int data = -1,handler_t func = Funtion1)
    {
        _data = data;
        _handler = func;
    }
    void Run()
    {
        _handler(_data);
    }
};

class PthreadPool
{
private:
    //线程数据任务队列
    std::queue<PthreadTask*> _queue;
    //线程
    pthread_t _tid[PTHREADNUM];
    //表示线程池是否要退出的标志
    bool _isQuit;
    //线程的现有个数
    int _pthreadSize;
    //线程的容量
    int _pthreadCapacity;
    //线程队列
    pthread_cond_t _threadCond;
    pthread_mutex_t _mutex;
    static void* ThreadStart(void* arg)
    {
        PthreadPool* tp = (PthreadPool*)arg;
        //直到最后主函数调用Quit才会退出
        while(1)
        {
            pthread_mutex_lock(&tp->_mutex);
            //如果没有任务要处理，则有两种选择
            while(tp->_queue.empty())
            {
                //1、如果要退出了，就还锁，退出
                if(tp->_isQuit)
                {
                    --tp->_pthreadSize;
                    pthread_mutex_unlock(&tp->_mutex);
                    pthread_exit(NULL);
                }
                //2、在PCB等待队列等待被唤醒
                pthread_cond_wait(&tp->_threadCond,&tp->_mutex);
            }
            PthreadTask* tt;
            tp->Pop(&tt);
            pthread_mutex_unlock(&tp->_mutex);
            //把任务拿到锁外面来跑，否则其他线程还得等这个任务跑完才能取锁，变成串行了
            tt->Run();
            delete tt;
        }
        return NULL;
    }
public:
    PthreadPool()
        :_pthreadSize(PTHREADNUM)
         ,_pthreadCapacity(PTHREADNUM)
    {
        _isQuit = false;
        pthread_cond_init(&_threadCond,NULL);
        pthread_mutex_init(&_mutex,NULL);
        Pthread_init();
    }
    ~PthreadPool()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_threadCond);
    }
    bool Pthread_init()
    {
        for(int i = 0; i < PTHREADNUM; ++i)
        {
            int ret = pthread_create(&_tid[i],NULL,ThreadStart,(void*)this);
            if(ret != 0)
            {
                std::cout << "pthread init failed!" << std::endl;
                return false;
            }
        }
        return true;
    }
    bool Push(PthreadTask* tt)
    {
        pthread_mutex_lock(&_mutex);
        if(_isQuit)
        {
            //退出前记得解锁
            pthread_mutex_unlock(&_mutex);
            return false;
        }
        _queue.push(tt);
        pthread_mutex_unlock(&_mutex);
        //push一个task就通知一个线程
        pthread_cond_signal(&_threadCond);
        return true;
    }
    bool Pop(PthreadTask** tt)
    {
        //因为改变的是指针，所以要传指针的地址
        *tt = _queue.front();
        _queue.pop();
        return true;
    }
    void Quit()
    {
        //这里加锁是为了让还在执行的线程把任务执行完
        //如果不加锁，可能刚刚线程就拿到锁就退出了
        //如果加了锁，线程刚拿到锁，这里是不能抢锁去置_isQuit为true的，要等线程把任务完成才能抢锁
        pthread_mutex_lock(&_mutex);
        _isQuit = true;
        pthread_mutex_unlock(&_mutex);
        //要用while去判断，否则只是通知一下就结束Quit这个函数了(通知只是一瞬间的事，并不会等待线程)
        //可能会退出的时候，线程还没执行完任务
        //所以要等线程一个都没有的时候才去退出
        while(_pthreadSize > 0)
        {
            pthread_cond_broadcast(&_threadCond);
        }
    }
};


int main()
{
    PthreadPool tp;
    for(int i = 0; i < 20; ++i)
    {
        PthreadTask* tt = new PthreadTask(i,Funtion1);
        tp.Push(tt);
    }
    //tp.Quit();
    return 0;
}
