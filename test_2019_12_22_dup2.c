#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
int main()
{
    int fd = open("tmp.txt",O_RDWR | O_CREAT,0664);
    if(fd < 0)
    {
        perror("open");
        return 0;
    }
    dup2(fd,1);
    printf("%s\n","hehe");

    return 0;
}

