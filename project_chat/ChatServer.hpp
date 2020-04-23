#pragma once
#include<string>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<pthread.h>
#include<iostream>

#include"MsgPool.hpp"
#include"LogSvr.hpp"
#include"ConnectInfo.hpp"
#include"UserManager.hpp"

#define UDP_PORT 4418
#define TCP_PORT 4419
#define THREAD_COUNT 2


class ChatServer
{
    private:
        //UDP用来在客户端之间接受消息
        int _udp_sock;
        int _udp_port;
        //TCP用来给客户注册和登陆
        int _tcp_sock;
        int _tcp_port;
        //数据池
        MsgPool* _msg_pool;
        //用户信息管理
        UserManager* _user_manager;
    public:
        ChatServer()
        {
            _udp_sock = -1;
            _udp_port = UDP_PORT;
            _tcp_sock = -1;
            _tcp_port = TCP_PORT;
            _msg_pool = NULL;
            _user_manager = NULL;
        }
        ~ChatServer()
        {
            if(_msg_pool)
            {
                delete _msg_pool;
                _msg_pool = NULL;
            }
            if(_user_manager)
            {
                delete _user_manager;
                _user_manager = NULL;
            }
        }
    //初始化成员变量
    void InitServer()
    {
        //1、创建UDP-SOCKET
        _udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); 
        if(_udp_sock < 0)
        {
            LOG(FATAL,"Create udp socket failed!");
            exit(1);
        }
        
        //绑定地址信息
        struct sockaddr_in udp_addr;
        udp_addr.sin_family = AF_INET;
        udp_addr.sin_port = htons(_udp_port);
        udp_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

        int ret = bind(_udp_sock,(struct sockaddr*)&udp_addr,sizeof(udp_addr));
        if(ret < 0)
        {
            LOG(FATAL,"Bind udp socket failed");
            exit(2);
        }
        LOG(INFO,"Udp bind success!");
        //2、创建TCP-SOCKET
        _tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_tcp_sock < 0)
        {
            LOG(FATAL,"Create tcp socket failed!");
            exit(3);
        }
        //绑定地址信息
        struct sockaddr_in tcp_addr;
        tcp_addr.sin_family = AF_INET;
        tcp_addr.sin_port = htons(_tcp_port);
        tcp_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        ret = bind(_tcp_sock,(struct sockaddr*)&tcp_addr,sizeof(tcp_addr));
        if(ret < 0)
        {
            LOG(FATAL,"Bind tcp socket failed!");
            exit(4);
        }
        //监听
        ret = listen(_tcp_sock,5);
        if(ret < 0)
        {
            LOG(FATAL,"Tcp listen failed!");
            exit(5);
        }
        LOG(INFO,"Tcp listen 0.0.0.0:4419");
        //3、初始化数据池
        _msg_pool = new MsgPool();
        if(_msg_pool == NULL)
        {
            LOG(FATAL,"Create msg pool failed!");
            exit(6);
        }
        LOG(INFO,"Create msg pool success!");
        //4、初始化用户管理
        _user_manager = new UserManager();
        if(_user_manager == NULL)
        {
            LOG(FATAL,"Create UserManager failed!");
            exit(7);
        }
    }
    //创建：生产者、消费者、登陆\注册 的线程
    void Start()
    {
        for(int i = 0; i < THREAD_COUNT; ++i)
        {
            pthread_t tid;
            int ret = pthread_create(&tid,NULL,ProductMsgStart,(void*)this);
            if(ret < 0)
            {
                LOG(FATAL,"product thread create failed!");
                exit(13);
            }
            ret = pthread_create(&tid,NULL,ConsumeMsgStart,(void*)this);
            if(ret < 0)
            {
                LOG(FATAL,"consume thread create failed!");
                exit(14);
            }
        }
        LOG(INFO,"Udp ChatServer start success!");
        while(1)
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int new_sock = accept(_tcp_sock,(struct sockaddr*)&cli_addr,&cli_len);
            if(new_sock < 0)
            {
                LOG(ERROR,"Accept new connect failed!");
                continue;
            }
            //一个new_sock新的连接处理一个登陆/注册的请求
            LoginConnect* lc = new LoginConnect(new_sock,(void*)this);
            if(lc == NULL)
            {
                LOG(ERROR,"Create LonginConnect failed!");
                continue;
            }
            //每接待到一个客户，就创建一个线程去处理登陆/注册
            pthread_t tid;
            int ret = pthread_create(&tid,NULL,LoginRegistStart,(void*)lc);
            if(ret < 0)
            {
                LOG(ERROR,"Create LoginRegistStart thread failed!");
                continue;
            }
            LOG(ERROR,"Create LoginRegistStart thread success!");
        }
    }
private:
    //主线程：处理客户的登陆和注册
    static void* LoginRegistStart(void* arg)
    {
        pthread_detach(pthread_self());
        LoginConnect* lc = (LoginConnect*)arg;
        int new_sock = lc->GetTcpSock();
        ChatServer* cs = (ChatServer*)(lc->GetServer());
        
        //先接受客户端的请求：登陆/注册(通过下面的request判断)
        char request;
        ssize_t recv_size = recv(new_sock,&request,1,0);
        if(recv_size < 0)
        {
            LOG(ERROR,"Recv request failed!");
            return NULL;
        }
        else if(recv_size == 0)
        {
            LOG(ERROR,"Client shutdown connect!");
            return NULL;
        }
        //解析客户的请求：注册、登陆、下线
        int user_id;
        int user_status = -1;
        switch(request)
        {
            case LOGIN:
                user_status = cs->DealLogin();
                break;
            case REGISTER:
                user_status = cs->DealRegister(lc->GetTcpSock(),&user_id);
                break;
            case LOGINOUT:
                user_status = cs->DealLoginOut();
                break;
            default:
                LOG(ERROR,"Received a not effective request!");
                break;
        }
        return NULL;
    }
    int DealRegister(int sock,int* user_id)
    {
        RegestInfo ri;
        ssize_t recv_size = recv(sock,&ri,sizeof(ri),0);
        if(recv_size < 0)
        {
            LOG(ERROR,"Recv RegestInfo failed!");
            return ONLINE; 
        }
        else if(recv_size == 0)
        {
            LOG(ERROR,"Client shutdown connect");
        }
        _user_manager->Register(); 
    }
    int DealLogin()
    {

    }
    int DealLoginOut()
    {

    }

    //将A客户的消息放在数据池里
    static void* ProductMsgStart(void* arg)
    {
       pthread_detach(pthread_self()); 
       ChatServer* cs = (ChatServer*)arg;
       while(1)
       {
           cs->RecvMsg();
       }
       return NULL;
    }
    //将线程池的数据发送给B客户
    static void* ConsumeMsgStart(void* arg)
    {
       pthread_detach(pthread_self()); 
       ChatServer* cs = (ChatServer*)arg;
       while(1)
       {
           cs->BroadcastMsg();
       }
       return NULL;
    }
    
private:
    void RecvMsg()
    {
        char buf[10240] = {0};
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int recv_size = recvfrom(_udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&cli_addr,&cli_len);
        if(recv_size < 0)
        {
            LOG(ERROR,"recvfrom msg failed!");
        }
        else
        {
            //正常逻辑
            std::string msg;
            msg.assign(buf,recv_size);
            LOG(INFO,msg);
            _msg_pool->PushMsgToPool(msg);
        }
    }
    void BroadcastMsg()
    {
        //1、获取要给哪一个用户发送
        //2、获取发送的内容
        //SendMsg();
        std::string msg;
        _msg_pool->PopMsgFromPool(&msg);
    }
    //给客户端发送一个消息
    void SendMsg(const std::string& msg,struct sockaddr_in& cli_addr,socklen_t& len)
    {
        int send_size = sendto(_udp_sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&cli_addr,len);
        if(send_size < 0)
        {
            LOG(ERROR,"send msg failed!");
        }
        else
        {

        }

    }

};
