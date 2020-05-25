#include<sys/epoll.h>
#include<unistd.h>
#include<stdio.h>

int main()
{
    //创建epoll套接字
    int epoll_fd = epoll_create(10);
    if(epoll_fd < 0)
    {
        perror("epoll_create");
        return -1;
    }

    //创建事件结构
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = 0;

    //将该事件结构添加到epoll中
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,0,&ev);

    while(1)
    {
        //先创建事件结构数组（是为了接收已经就绪的事件）
        struct epoll_event event_arr[10];
        int ret = epoll_wait(epoll_fd,event_arr,sizeof(event_arr)/sizeof(event_arr[0]),300);

        if(ret < 0)
        {
            perror("epoll_wait");
            return -1;
        }
        else if(ret == 0)
        {
            printf("epoll_wait time out!\n");
            continue;
        }
        
        //接下来遍历就绪的事件数组
        for(size_t i = 0; i < sizeof(event_arr); ++i)
        {
            if(event_arr[i].data.fd == 0)
            {
                char buf[1024] = {0};
                read(event_arr[i].data.fd,buf,sizeof(buf)-1);
                printf("我们输入的是：%s\n",buf);
            }
        }
        
    }


    return 0;
}
