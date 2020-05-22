#pragma once
#include<sys/select.h>
#include<vector>

#include"tcp_socket.hpp"

class SelectServer
{
    private:
        fd_set _read_fds;
        int _max_fd;
    public:
        SelectServer()
        {
            FD_ZERO(&_read_fds);
            _max_fd = -1;
        }
        void AddFd(int fd)
        {
            if(fd > _max_fd)
            {
                _max_fd = fd;
            }
            FD_SET(fd,&_read_fds);
        }
        void DeleteFd(int fd)
        {
            FD_CLR(fd,&_read_fds);

            //如果删除的是最大的文件描述符，则需要再次从后往前遍历的寻找一个当前最大的fd
            if(fd == _max_fd)
            {
                while(!FD_ISSET(_max_fd,&_read_fds))
                {
                   --_max_fd;
                }
            }
        }
        bool WaitFd(std::vector<TcpSocket>* vec)
        {
            struct timeval tv; 
            tv.tv_sec = 3;

            fd_set tmp_fds = _read_fds;
            int ret = select(_max_fd+1,&tmp_fds,NULL,NULL,&tv);
            if(ret < 0)
            {
                perror("select");
                return false;
            }
            else if(ret == 0)
            {
                printf("select time out\n");
                return false;
            }
            for(int i = 0; i <= _max_fd; ++i)
            {
                if(FD_ISSET(i,&tmp_fds))
                {
                    TcpSocket ts;
                    ts.SetFd(i);
                    vec->push_back(ts);
                }
            }
            return true; 
        }
};
