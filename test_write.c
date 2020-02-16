#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
    int fd = open("fifo",O_WRONLY);
    if(fd < 0)
    {
        perror("open failed!");
        return -1;
    }
    printf("kaishi\n");
    char buf[20] = "shishuai";
    int ret = write(fd,buf,8);
    if(ret < 0)
    {
        printf("写失败了\n");
    }
    printf("写成功了：%s\n",buf);
    printf("ret = %d\n",ret);
    close(fd);
    return 0;
}



