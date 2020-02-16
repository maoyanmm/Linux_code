#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
    int fd = open("fifo",O_RDONLY);
    if(fd < 0)
    {
        perror("open failed!");
        return -1;
    }
    char buf[20] = {0};
    //int ret = read(fd,buf,20);
    printf("读成功了：%s\n",buf);
    //printf("ret = %d\n",ret);
    close(fd);

    return 0;
}
