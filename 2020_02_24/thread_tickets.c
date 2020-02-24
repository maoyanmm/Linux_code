#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

int g_tickets = 100;
pthread_mutex_t mutex;

void* get_tickets(void* arg)
{
    char* id = (char*)arg;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        if(g_tickets > 0)
        {
            usleep(10000);
            --g_tickets;
            printf("the [%s] have got ticket num is [%d]\n",id,g_tickets+1);
        }
        else
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main()
{
    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;
    pthread_t tid4;
    pthread_mutex_init(&mutex,NULL);
    pthread_create(&tid1,NULL,get_tickets,"thread1");
    pthread_create(&tid2,NULL,get_tickets,"thread2");
    pthread_create(&tid3,NULL,get_tickets,"thread3");
    pthread_create(&tid4,NULL,get_tickets,"thread4");

    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    pthread_join(tid3,NULL);
    pthread_join(tid4,NULL);
    return 0;
}
