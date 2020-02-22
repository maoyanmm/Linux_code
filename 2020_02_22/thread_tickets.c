#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

int global_tickets = 100;

void* get_tickets(void* arg)
{
    (void)arg;
    while(global_tickets > 0)
    {
        usleep(100000);
        printf("thread[%p] have got ticket number is[%d]\n",pthread_self(),global_tickets);
        --global_tickets;
    }
    return NULL;
}

int main()
{
    pthread_t tid[4];
    int i = 0;
    for(i = 0; i < 4; ++i)
    {
        pthread_create(&tid[i],NULL,get_tickets,NULL);
    }
    pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    pthread_join(tid[2],NULL);
    pthread_join(tid[3],NULL);

    return 0;
}
