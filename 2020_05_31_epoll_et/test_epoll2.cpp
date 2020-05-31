#include<sys/epoll.h>
#include<stdio.h>
#include<iostream>
#include<string>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
using namespace std;

int main()
{
    //一定要设置成非阻塞的：
    //如果是阻塞的，则可能会一直阻塞在read读
    //容量是5的BUF(实际装4个)，如果读了4个了，然后再循环过来的时候，缓冲区是没有数据的，所以会阻塞
    int flag = fcntl(0,F_GETFL);
    fcntl(0,F_SETFL,flag | O_NONBLOCK);

    //1、创建epoll套接字
    int ep_fd = epoll_create(1);
    
    //2、添加事件
    struct epoll_event ev;
    ev.data.fd = 0;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(ep_fd,EPOLL_CTL_ADD,0,&ev);

    //3、开始监听
    while(1)
    {
        struct epoll_event event_arr[10];
        int ret = epoll_wait(ep_fd,event_arr,sizeof(event_arr)/sizeof(event_arr[0]),-1);
        if(ret < 0)
        {
            perror("epoll_wait");
            continue;
        }
        if(ret == 0)
        {
            cout << "time out " << endl;
            continue;
        }
        //遍历就绪的文件描述符
        for(int i = 0; i < ret; ++i)
        {
            if(event_arr[i].data.fd == 0)
            {
                std::string read_str;
                while(1)
                {
                    char buf[5] = {0};
                
                    cout << "============="  << endl;
                    ssize_t recv_sz = read(0,buf,sizeof(buf)-1);
                    cout << "============="  << endl;
                    if(recv_sz < 0)
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            goto myend;
                        }
                        perror("read");
                        return 0;
                    }

                    read_str += buf;

                    if(recv_sz < (ssize_t)sizeof(buf)-1)
                    {
myend:
                        cout << "刚刚输入的是：" << read_str << endl;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
