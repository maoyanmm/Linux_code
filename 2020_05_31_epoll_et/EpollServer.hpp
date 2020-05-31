#include<unistd.h>
#include<stdio.h>
#include<iostream>
#include<sys/epoll.h>
#include<vector>
using namespace std;

#include"tcp_socket.hpp"

class EpollServer
{
    private:
        int _epoll_fd;
    public:
        EpollServer()
        {
            //创建epoll套接字
            _epoll_fd = epoll_create(10);
        }
        ~EpollServer()
        {
            if(_epoll_fd >= 0)
            {
                close(_epoll_fd);
            }
        }
        bool AddFd(int fd, bool et_opt = false)
        {
            //创建一个事件结构，并把输入的fd传入
            struct epoll_event ev;
            //看是否需要设置ET模式(默认是LT模式)
            if(et_opt == true)
            {
                ev.events = EPOLLIN | EPOLLET;
            }
            else
            {
                ev.events = EPOLLIN;
            }
            ev.data.fd = fd;
            
            //将这个时间传入event_epoll
            int ret = epoll_ctl(_epoll_fd,EPOLL_CTL_ADD,fd,&ev);
            if(ret < 0)
            {
                perror("epoll_ctl add");
                return false;
            }
            return true;
        }
        bool DeleteFd(int fd)
        {
            //删除的时候不需要再传入事件的结构体了
            int ret = epoll_ctl(_epoll_fd,EPOLL_CTL_DEL,fd,NULL);
            if(ret < 0)
            {
                perror("epoll_ctl delete");
                return false;
            }
            return true;
        }
        //将就绪的文件描述符通过指针传出
        bool WaitFd(std::vector<TcpSocket>* vec)
        {
            struct epoll_event event_arr[100];
            int ret = epoll_wait(_epoll_fd,event_arr,sizeof(event_arr)/sizeof(event_arr[0]),-1);
            if(ret < 0)
            {
                perror("epoll_wait");
                return false;
            }

            //这里只需要遍历ret个数就可以了
            for(int i = 0; i < ret; ++i)
            {
                int ready_fd = event_arr[i].data.fd;
                TcpSocket ts;
                ts.SetFd(ready_fd);
                vec->push_back(ts);
            }
            return true;
        }

    
};
