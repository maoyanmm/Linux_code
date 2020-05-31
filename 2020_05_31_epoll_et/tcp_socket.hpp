#include<iostream>
#include<fcntl.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string>
#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>

class TcpSocket
{
private:
    int _fd;
public:
    TcpSocket()
        :_fd(-1)
    {}
    void SetFd(int fd)
    {
        _fd = fd;
    }
    int GetFd()
    {
        return _fd;
    }
    bool Socket()
    {
        _fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_fd < 0)
        {
            perror("Socket");
            return false;
        }
        printf("open socket fd = [%d]\n",_fd);
        return true;
    }
    bool Bind(const std::string& ip, uint16_t port)const
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port = htons(port);

        int ret = bind(_fd,(struct sockaddr*)&addr,sizeof(addr));
        if(ret < 0)
        {
            perror("Bind");
            return false;
        }
        printf("bind ip[%s] port[%d]\n",ip.c_str(),port);
        return true;
    }
    bool Listen(int backlog = 5)const
    {
        int ret = listen(_fd,backlog);
        if(ret < 0)
        {
            perror("Listen");
            return false;
        }
        return true;
    }
    bool Connect(std::string& ip, uint16_t port)
    {
       struct sockaddr_in svrAddr;
       svrAddr.sin_family = AF_INET;
       svrAddr.sin_addr.s_addr = inet_addr(ip.c_str());
       svrAddr.sin_port = htons(port);

       int ret = connect(_fd,(struct sockaddr*)&svrAddr,sizeof(svrAddr));
       if(ret < 0)
       {
           perror("Connect");
           return false;
       }
       printf("Connected ip[%s] port[%d]\n",ip.c_str(),port);
       return true;
    }
    bool Accept(TcpSocket* peer_sock, std::string* ip = NULL, uint16_t* port = NULL)
    {
        struct sockaddr_in peer_addr;
        socklen_t addrLen = sizeof(peer_addr);
        int newFd = accept(_fd,(struct sockaddr*)&peer_addr,&addrLen);
        if(newFd < 0)
        {
            perror("Accept");
            return false;
        }
        printf("accept fd = [%d]",newFd);
        peer_sock->_fd = newFd;
        if(ip != NULL)
        {
            *ip = inet_ntoa(peer_addr.sin_addr);
        }
        if(port != NULL)
        {
            *port = ntohs(peer_addr.sin_port);
        }
        return true;
    }
    bool Send(std::string& buf)const
    {
       int sendSize = send(_fd,buf.c_str(),buf.size(),0);
       if(sendSize < 0)
       {
           printf("Send failed!!!\n");
           return false;
       }
       return true;
    }
    bool Recv(std::string& buf)const
    {
        char tmp[1024] = {0};
        int readSize = recv(_fd,tmp,sizeof(tmp)-1,0);    
        if(readSize < 0)
        {
            perror("Recv");
            return false;
        } else if(readSize == 0)
        {
            return false;
        }
        buf.assign(tmp,readSize);
        return true;

    }
    void Close()
    {
        close(_fd);
        printf("close fd[_%d]",_fd);
    }
    //将文件描述符设置为非阻塞
    bool SetNonBlock()
    {
        int flag = fcntl(_fd,F_GETFL);
        int ret = fcntl(_fd,F_SETFL,flag | O_NONBLOCK);
        if(ret < 0)
        {
            return false;
        }
        return true;
    }
    //非阻塞的接收数据
    bool RecvNonBlock(std::string* recv_data)
    {
        while(1)
        {
            char buf[5] = {0};
            int recv_sz = recv(_fd,buf,sizeof(buf)-1,0);
            //如果recv_sz小于零，则有两种可能：
            //1、出现了错误
            //2、没有数据可以接受了
            if(recv_sz < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                perror("recv");
                return false;
            }
            //如果recv_sz等于零，表示对端关闭了连接
            else if(recv_sz == 0)
            {
                std::cout << "Peer shut down!" << std::endl;
                return false;
            }

            //正常接受到数据，则拼接
            *recv_data += buf;
            //如果接受正常，判断本次接受的数据是否是最后的数据
            if(recv_sz < (int)sizeof(buf)-1)
            {
                break;     
            }
        }
        return true;
    }
    //非阻塞的发送数据
    bool SendNonBlock(const std::string& send_data)
    {
        //表示当前要发送数据的起始下标
        int index = 0;
        //表示要发送的剩余数据的大小
        int remain_sz = send_data.size();
        while(1)
        {
            int send_sz = send(_fd, send_data.c_str()+index, remain_sz, 0); 
            if(send_sz < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                perror("send");
                return false;
            }

            //每次发完一部分数据，起始的下标都增加，剩余的空间都减小
            index += send_sz;
            remain_sz -= send_sz;
            
            //如果剩余的数据没有了，则发完了
            if(remain_sz <= 0)
            {
                break;
            }
        }
        return true;
    }
}
;
