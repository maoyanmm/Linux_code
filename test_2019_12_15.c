#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
int main()
{
    pid_t pid = fork();
    if(0 == pid)
    {
        printf("i am child:[%d]\n",getpid());
        sleep(3);
        exit(1);
    }
    else if(pid < 0)
    {
        perror("perro");
    }
    else
    {
        pid_t pid2 = fork();
        if(pid2 == 0)
        {
            printf("child2:[%d]",getpid());
            exit(1);
        }
        printf("i am parent:[%d]\n",getpid());

        waitpid(pid,NULL,0);
        while(1)
        {
            ;
        }
    }
    return 0;
}
