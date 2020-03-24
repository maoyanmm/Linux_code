#pragma once
#include<iostream>
#include<string>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include"DataPool.hpp"
#include"Log.hpp"

#define UDP_PORT 4418
#define THREAD_COUNT 2

class ChatServer
{
private:
    int _udp_sock;
    int _udp_port;
    DataPool* _data_pool; 
public:
    ChatServer()
    {
        _udp_port = UDP_PORT;
        _udp_sock = -1;
        _data_pool = NULL;
    }
    ~ChatServer()
    {
        close(_udp_sock);  
        //如果创建了数据池，则delete
        if(_data_pool != NULL)
        {
            delete(_data_pool);
        }
    }
    void InitServer()
    {
        //创建套接字
        _udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); 
        if(_udp_sock < 0)
        {
            perror("socket");
            exit(1);
        }
        //绑定套接字 
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_udp_port);
        addr.sin_addr.s_addr = inet_addr("0.0.0.0");//绑定主机任意一个ip

        int ret = bind(_udp_sock,(struct sockaddr*)&addr,sizeof(addr));
        if(ret < 0)
        {
            perror("bind");
            exit(2);
        }
        //初始化数据池
        _data_pool = new DataPool;
        if(_data_pool == NULL)
        {
            perror("create DataPool failed");
            exit(3);
        }
    }
    void Start()
    {
        pthread_t tid;
        for(int i = 0; i < THREAD_COUNT; ++i)
        {
            //创建生产线程
            int ret = pthread_create(&tid,NULL,ProductDataStart,(void*)this);
            if(ret < 0)
            {
                perror("pthread_create");
                exit(4);
            }
            //创建消费线程
            ret = pthread_create(&tid,NULL,ConsumeDataStart,(void*)this);
            if(ret < 0)
            {
                perror("pthread_create");
                exit(4);
            }
        }
    }
    static void* ProductDataStart(void* arg)
    {
        ChatServer* cs = (ChatServer*)arg;
        while(1)
        {
            cs->RcvData();
        }
        return NULL;
    }
    static void* ConsumeDataStart(void* arg)
    {
        ChatServer* cs = (ChatServer*)arg;
        while(1)
        {
            cs->BroadCast();
        }
        return NULL;
    }
private:
    void RcvData()
    {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        char buf[10240] = {0};//用来临时接收数据
        int recv_size = recvfrom(_udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&addr,&addr_len);
        if(recv_size < 0)
        {
            perror("recvfrom");
        }
        else
        {
            std::string data;
            //将数据转成string
            data.assign(buf,recv_size);
            //将string数据放入数据池
            _data_pool->PushDataToPool(data);
        }
    }
    void BroadCast()
    {
        
    }
    void SendData(std::string& data, struct sockaddr_in& addr, socklen_t& len)
    {
        size_t send_size = sendto(_udp_sock,data.c_str(),data.size(),0,(struct sockaddr*)&addr,len);
    }
};
