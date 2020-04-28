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
#include"Message.hpp"
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
            LOG(FATAL,"Create udp socket failed!") << std::endl;
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
            LOG(FATAL,"Bind udp socket failed") << std::endl;
            exit(2);
        }
        LOG(INFO,"Udp bind success!") << std::endl;
        //2、创建TCP-SOCKET
        _tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(_tcp_sock < 0)
        {
            LOG(FATAL,"Create tcp socket failed!") << std::endl;
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
            LOG(FATAL,"Bind tcp socket failed!") << std::endl;
            exit(4);
        }
        //监听
        ret = listen(_tcp_sock,5);
        if(ret < 0)
        {
            LOG(FATAL,"Tcp listen failed!") << std::endl;
            exit(5);
        }
        LOG(INFO,"Tcp listen 0.0.0.0:4419") << std::endl;
        //3、初始化数据池
        _msg_pool = new MsgPool();
        if(_msg_pool == NULL)
        {
            LOG(FATAL,"Create msg pool failed!") << std::endl;
            exit(6);
        }
        LOG(INFO,"Create msg pool success!") << std::endl;
        //4、初始化用户管理
        _user_manager = new UserManager();
        if(_user_manager == NULL)
        {
            LOG(FATAL,"Create UserManager failed!") << std::endl;
            exit(7);
        }
    }
    //创建：生产者、消费者、登陆\注册 的线程
    void Start()
    {
        //创建生产者消费者线程
        for(int i = 0; i < THREAD_COUNT; ++i)
        {
            pthread_t tid;
            int ret = pthread_create(&tid,NULL,ProductMsgStart,(void*)this);
            if(ret < 0)
            {
                LOG(FATAL,"product thread create failed!") << std::endl;
                exit(13);
            }
            ret = pthread_create(&tid,NULL,ConsumeMsgStart,(void*)this);
            if(ret < 0)
            {
                LOG(FATAL,"consume thread create failed!") << std::endl;
                exit(14);
            }
        }
        LOG(INFO,"Udp ChatServer start success!") << std::endl;
        //创建登陆/注册线程
        while(1)
        {
            //1、接受一个客户的tcp
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int new_sock = accept(_tcp_sock,(struct sockaddr*)&cli_addr,&cli_len);
            if(new_sock < 0)
            {
                LOG(ERROR,"Accept new connect failed!") << std::endl;
                continue;
            }
            //2、一个new_sock新的连接处理一个登陆/注册的请求
            LoginConnect* lc = new LoginConnect(new_sock,(void*)this);
            if(lc == NULL)
            {
                LOG(ERROR,"Create LonginConnect failed!") << std::endl;
                continue;
            }
            //3、每接待到一个客户，就创建一个线程去处理登陆/注册
            pthread_t tid;
            int ret = pthread_create(&tid,NULL,LoginRegistStart,(void*)lc);
            if(ret < 0)
            {
                LOG(ERROR,"Create LoginRegistStart thread failed!") << std::endl;
                continue;
            }
            LOG(INFO,"Create LoginRegistStart thread success!") << std::endl;
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
            LOG(ERROR,"Recv request failed!") << std::endl;
            return NULL;
        }
        else if(recv_size == 0)
        {
            LOG(ERROR,"Client shutdown connect!") << std::endl;
            return NULL;
        }
        //解析客户的请求：注册、登陆、下线
        uint64_t user_id = -1;
        int user_status = -1;
        switch(request)
        {
            case LOGIN:
                user_status = cs->DealLogin(lc->GetTcpSock());
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
        ReplyInfo ri;
        ri._status = user_status;
        ri._user_id = user_id;
        ssize_t send_size = send(lc->GetTcpSock(), &ri, sizeof(ri), 0);
        if(send_size < 0)
        {
            LOG(ERROR,"Send ReplyInfo failed!") << std::endl;
            return NULL;
        }
        LOG(INFO,"Send ReplyInfo success!") << std::endl;

        close(lc->GetTcpSock());
        delete lc;
        return NULL;
    }

    int DealRegister(int sock,uint64_t* user_id)
    {
        RegestInfo ri;
        ssize_t recv_size = recv(sock,&ri,sizeof(ri),0);
        if(recv_size < 0)
        {
            LOG(ERROR,"Recv RegestInfo failed!") << std::endl;
            return OFFLINE; 
        }
        else if(recv_size == 0)
        {
            LOG(ERROR,"Client shutdown connect") << std::endl;
        }
        int ret = _user_manager->Register(ri._nick_name,ri._school,ri._password,user_id); 
        if(ret == -1)
        {
            return REGIST_FAILED;
        }
        return REGIST_SUCCESS;
    }
    int DealLogin(int sock)
    {
        LoginInfo li;
        ssize_t recv_size = recv(sock,&li,sizeof(li),0);
        if(recv_size < 0)
        {
            LOG(ERROR,"Recv LoginInfo failed!") << std::endl;
            return OFFLINE; 
        }
        else if(recv_size == 0)
        {
            LOG(ERROR,"Client shutdown connect") << std::endl;
        }
        int ret = _user_manager->Login(li._user_id,li._password);
        if(ret == -1)
        {
            return LOGIN_FAILED;
        }
        return LOGIN_SUCCESS;
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
            LOG(ERROR,"recvfrom msg failed!") << std::endl;
        }
        else
        {
            std::string msg;
            msg.assign(buf,recv_size);
            LOG(INFO,msg) << std::endl;
            Message json_msg;
            json_msg.Deserialize(msg);
            
            //不是所有的消息都接收到消息池，可以接受的消息：
            //1、新用户发送了注册请求
            //2、老用户发送了登陆请求、发送的消息
            bool ret = _user_manager->IsLogin(json_msg.GetUserId(),cli_addr,cli_len);
            if(ret == false)
            {
                LOG(ERROR,"discarded the msg") << std::endl;
                return;
            }
            LOG(INFO,"PushMsgToPool success!") << std::endl;
            _msg_pool->PushMsgToPool(msg);
        }
    }
    void BroadcastMsg()
    {
        //1、获取要发送的消息
        std::string msg;
        _msg_pool->PopMsgFromPool(&msg);
        //2、获取所有在线用户的列表
        std::vector<UserInfo> online_list;
        _user_manager->GetOnlineUser(&online_list);
        //3、遍历在线列表一个个发送
        std::vector<UserInfo>::iterator it = online_list.begin();
        for(; it != online_list.end(); ++it)
        {
            SendMsg(msg,it->GetCliAddr(),it->GetCliAddrLen());
        }
    }
    //给客户端发送一个消息
    void SendMsg(const std::string& msg,struct sockaddr_in& cli_addr,socklen_t& len)
    {
        int send_size = sendto(_udp_sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&cli_addr,len);
        if(send_size < 0)
        {
            LOG(ERROR,"send msg failed!") << std::endl;
        }
        else
        {
            LOG(INFO,"SendMsg success ") <<  "[" << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << "]" << std::endl;
        }

    }
};
