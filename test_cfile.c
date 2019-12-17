#include<stdio.h>
#include<string.h>
int main()
{
    while(1)
    {

    FILE* fd = fopen("tmp.txt","w+");
    if(!fd)
    {
        perror("fopen");
        return 0;
    }
    }
    FILE* fd = fopen("tmp.txt","w+");
    if(!fd)
    {
        perror("fopen");
        return -1;
    }
    printf("fopen success\n");
    const char* arr = "shishuainb";
    fwrite(arr,1,strlen(arr),fd);
    //fseek(fd,0,SEEK_SET);

    char buffer[1024] = {0};
    int size = fread(buffer,1,sizeof(buffer)-1,fd);
    if(size <= 0)
    {
        perror("fread");
        return -1;

    }
    printf("%s\n",buffer);
    return 0;
}
