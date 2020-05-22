#include<iostream>
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
           perror("Send");
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
};
