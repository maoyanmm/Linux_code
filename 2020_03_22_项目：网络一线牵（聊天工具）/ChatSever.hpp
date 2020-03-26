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
#include"ConnectInfo.hpp"
#include"UserManager.hpp"

#define UDP_PORT 4418
#define TCP_PORT 4428
#define THREAD_COUNT 2
#define LISTEN_NUM 5

//该类用来收发数据给客户端
class ChatServer
{
private:
    //udp是用来发送消息的套接字
    int _udp_sock;
    int _udp_port;
    //tcp是用来给用户注册的套接字
    int _tcp_sock;
    int _tcp_port;
    //数据池，用来存放接收到的客户端的数据，然后发给其他客户端
    DataPool* _data_pool; 
    //用来存放用户的各种信息
    UserManager* _usr_manager; 

public:
    ChatServer()
    {
        _udp_port = UDP_PORT;
        _udp_sock = -1;
        _data_pool = NULL;
        _usr_manager = NULL;
        _tcp_port = TCP_PORT;
        _tcp_sock = -1;
    }
    ~ChatServer()
    {
        close(_udp_sock);  
        close(_tcp_sock);
        //如果创建了数据池，则delete
        if(_data_pool != NULL)
        {
            delete(_data_pool);
            _data_pool = NULL;
        }
        if(_usr_manager != NULL)
        {
            delete _usr_manager;
            _usr_manager = NULL;
        }
    }
    //初始化私有成员
    void InitServer()
    {
        //1、创建套UDP接字，就是要收发数据的套接字
        _udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); 
        if(_udp_sock < 0)
        {
            LOG(FATAL,"socket create failed!");
            exit(1);
        }
        //绑定UDP套接字 
        struct sockaddr_in udp_addr;
        udp_addr.sin_family = AF_INET;
        udp_addr.sin_port = htons(_udp_port);
        udp_addr.sin_addr.s_addr = inet_addr("0.0.0.0");//绑定主机任意一个ip
        int ret = bind(_udp_sock,(struct sockaddr*)&udp_addr,sizeof(udp_addr));
        if(ret < 0)
        {
            LOG(FATAL,"udp socket bind failed!");
            exit(2);
        }
        LOG(INFO,"udp socket bind success!");

        //2、创建TCP套接字，也就是注册要用到的TCP
        _tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_tcp_sock < 0)
        {
            LOG(FATAL,"tcp socket create failed!");
            exit(3);
        }
        //绑定TCP套接字
        struct sockaddr_in tcp_addr;
        tcp_addr.sin_family = AF_INET;
        tcp_addr.sin_port = htons(TCP_PORT);
        tcp_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        ret = bind(_tcp_sock,(struct sockaddr*)&tcp_addr,sizeof(tcp_addr));
        if(ret < 0)
        {
            LOG(FATAL,"tcp socket bind failed!");
            exit(4);
        }
        //监听套接字
        ret = listen(_tcp_sock,LISTEN_NUM);
        if(ret < 0)
        {
            LOG(FATAL,"tcp listen failed!");
            exit(5);
        }

        //3、初始化数据池
        _data_pool = new DataPool;
        if(_data_pool == NULL)
        {
            LOG(FATAL,"datapool create failed!");
            exit(6);
        }
        LOG(INFO,"datapool create success!");
        
        //4、创建用户信息管理
        _usr_manager = new UserManager;
        if(_usr_manager == NULL)
        {
            LOG(FATAL,"UserManager create failed!");
            exit(7);
        }
    }
    //启动各个服务的线程
    void Start()
    {
        //启动生产者和消费者的线程
        pthread_t tid;
        for(int i = 0; i < THREAD_COUNT; ++i)
        {
            //创建生产线程
            int ret = pthread_create(&tid,NULL,ProductDataStart,(void*)this);
            if(ret < 0)
            {
                LOG(FATAL,"thread create failed!");
                exit(10);
            }
            //创建消费线程
            ret = pthread_create(&tid,NULL,ConsumeDataStart,(void*)this);
            if(ret < 0)
            {
                LOG(FATAL,"thread create failed!");
                exit(10);
            }
        }

        //启动登陆和注册的线程
        //反复的接收并处理：要注册或者登陆的用户
        while(1)
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_addrlen;
            int newsock = accept(_tcp_sock,(struct sockaddr*)&cli_addr,&cli_addrlen);
            if(newsock < 0)
            {
                LOG(ERROR,"accept failed!");
                continue;
            }
            LoginConnect* lc = new LoginConnect(newsock,(void*)this);
            if(lc == NULL)
            {
                LOG(ERROR,"create LoginConnect failed!");
                continue;
            }
            //开始创建线程去处理登陆/注册请求
            pthread_t tid;
            int ret = pthread_create(&tid,NULL,LoginRegisterStart,(void*)lc);
            if(ret < 0)
            {
                LOG(ERROR,"create LoginRegisterStart thread failed!");
                continue;
            }
        }
    }
private:
    static void* LoginRegisterStart(void* arg)
    {
        pthread_detach(pthread_self());
        LoginConnect* lc = (LoginConnect*)arg;
        //拿到lc里存的ChatServer的this指针
        ChatServer* cs = (ChatServer*)lc->GetServer();
        char quest;//用来接收用户发送的请求（登陆或者注册的请求）
        ssize_t recv_size = recv(lc->GetTcpSock(),&quest,1,0);
        if(recv_size < 0)
        {
            LOG(ERROR,"receive quest failed!");
            return NULL;
        }
        else if(recv_size == 0)
        {
            LOG(ERROR,"client shutdown connect!");
            return NULL;
        }
        uint64_t usr_id = -3;
        int usr_status = -3;
        //开始辨识请求是什么（登陆或注册请求）
        switch(quest)
        {
            case REGISTER:
                break;
            case LOGIN:
                break;
            case LOGINOUT:
                break;
            default:
                break;
        }
        return NULL;
    }
    void DealRigister(int sock, uint64_t& usr_id)
    {
      RegisterInfo ri;
      int recv_size = recv(sock,&ri,sizeof(ri),0);
      if(recv_size < 0)
      {
          LOG(ERROR,"receive RegisterInfo failed!");
      }
      else if(recv_size == 0)
      {
          LOG(ERROR,"client shutdown connect");
      }
      _usr_manager->Register();
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
