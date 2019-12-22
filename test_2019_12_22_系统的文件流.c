#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
int main()
{
    //打开文件
    //O_RDONLY 只读
    //O_WRONLY 只写
    //O_RDWR 读写
    //以上必须任选其一
    //
    //O_CREAT 如果打开的这个文件不存在则创建文件
    //O_TRUNC 打开文件后截断文件
    //O_APPEND 以追加方式打开
    int fd = open("./tmp.txt",O_RDWR | O_CREAT | O_APPEND,0664);
    if(fd < 0)
    {
        perror("open");
        return 0;
    }
    const char* str = "shishuai666";
    printf("fd:[%d]\n",fd);

    int ret = write(fd,str,strlen(str));
    if(ret < 0)
    {
        perror("write");
        return 0;
    }

    lseek(fd,0,SEEK_SET);

    char buf[100] = {0};
    ret = read(fd,buf,sizeof(buf)-1);
    if(ret < 0)
    {
        perror("read");
        return 0;
    }


    printf("%s\n",buf);
    close(fd);


    return 0;
}
