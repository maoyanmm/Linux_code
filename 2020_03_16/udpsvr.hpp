#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string>
#include<iostream>

class UdpSvr
{
private:
    int _sock;
public:
    UdpSvr()
        :_sock(-1)
    {}
    ~UdpSvr()
    {
    }

    bool Create_Socket()
    {
        _sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(_sock < 0)
        {
            perror("Create_Socket");
            return false;
        }
        return true;
    }
    bool Bind(std::string ip,uint16_t port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port); 
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        
        int ret = bind(_sock,(struct sockaddr*)&addr,sizeof(addr));
        if(ret < 0)
        {
            perror("Bind");
            return false;
        }
        return true;
    }
    bool Send_Message(std::string& buf,struct sockaddr_in* dstAddr)
    {
        int ret = sendto(_sock,buf.c_str(),buf.size(),0,(struct sockaddr*)dstAddr,sizeof(struct sockaddr_in));
        if(ret < 0)
        {
            perror("Send_Message");
            return false;
        }
        return true;
    }
    bool Receive_Message(std::string& buf,struct sockaddr_in* srcAddr)
    {
        char tmp[1024] = {0};
        socklen_t sockLen = sizeof(struct sockaddr_in);
        int recvSize = recvfrom(_sock,tmp,sizeof(tmp)-1,0,(struct sockaddr*)srcAddr,&sockLen);
        if(recvSize < 0)
        {
            perror("Receive_Message");
            return false;
        }
        buf.assign(tmp,recvSize);
        return true;
    }
    void Close()
    {
        close(_sock);
        _sock = -1;
    }
};
