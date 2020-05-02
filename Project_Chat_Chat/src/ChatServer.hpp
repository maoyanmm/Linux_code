#pragma once
#include<iostream>
#include<pthread.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>

#include"Log.hpp"
#include"UserManager.hpp"
#include"FormatMessage.hpp"
#include"MsgPool.hpp"
#include"ConnectInfo.hpp"

#define UDP_PORT 4418
#define TCP_PORT 4419
#define THREAD_NUM 2

class ChatServer;
//封装了new_sock 和 ChatServer ，为了传一个参数
class NewsockAndChatserver
{
    public:
        int _new_sock;
        ChatServer* _cs;
    public:
        NewsockAndChatserver(int new_sock,ChatServer* cs)
            :_new_sock(new_sock),_cs(cs)
        {  }
};

class ChatServer
{
    private:
        //发送消息用的udp
        int _udp_sock;
        uint16_t _udp_port;
        //登陆注册要用的tcp
        int _tcp_sock;
        uint16_t _tcp_port;
        //数据池
        MsgPool* _msg_pool; 
        //用户管理模块
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
            InitServer();
            Start();
        }
        ~ChatServer()
        {
            if(_msg_pool != NULL)
            {
                delete _msg_pool;
            }
            if(_user_manager != NULL)
            {
                delete _user_manager;
            }
        }
        void InitServer()
        {
            //1、初始化UDP
            //1.1、初始化udp套接字
            _udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
            if(_udp_sock < 0)
            {
                LOG(FATAL,"_udp_sock init failed!") << std::endl;
                exit(1);
            }
            //1.2、绑定地址
            struct sockaddr_in svr_udp_addr;
            svr_udp_addr.sin_family = AF_INET;
            svr_udp_addr.sin_port = htons(_udp_port);
            svr_udp_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
            
            int ret = bind(_udp_sock,(struct sockaddr*)&svr_udp_addr,sizeof(svr_udp_addr));
            if(ret < 0)
            {
                LOG(FATAL,"_udp bind failed!") << std::endl;
                exit(2);
            }
            LOG(INFO,"UDP init success!!!") << std::endl;
             
            //2、初始化TCP
            //2.1、初始化tcp套接字
            _tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            if(_tcp_sock < 0)
            {
                LOG(FATAL,"_tcp_sock init failed!") << std::endl;
                exit(3);
            }
            //2.2、绑定地址
            struct sockaddr_in svr_tcp_addr;
            svr_tcp_addr.sin_family = AF_INET;
            svr_tcp_addr.sin_port = htons(_tcp_port);
            svr_tcp_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
            //2.2.1、地址复用：服务器突然挂掉的时候可以立即绑定这个地址，否则要等2MSL
            int opt = 1;
            setsockopt(_tcp_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

            ret = bind(_tcp_sock,(struct sockaddr*)&svr_tcp_addr,sizeof(svr_tcp_addr));
            if(ret < 0)
            {
                LOG(FATAL,"_tcp bind failed!") << std::endl;
                exit(4);
            }
            //2.3、监听
            ret = listen(_tcp_sock,5);
            if(ret < 0)
            {
                LOG(FATAL,"_tcp listen failed!") << std::endl;
                exit(5);
            }
            LOG(INFO,"TCP init success!!!") << std::endl;

            //3、初始化数据池
            _msg_pool = new MsgPool();
            if(_msg_pool == NULL)
            {
                LOG(FATAL,"_msg_pool init failed!") << std::endl;
                exit(6);
            }
            LOG(INFO,"MsgPool init success!!!") << std::endl;

            //4、初始化用户管理
            _user_manager = new UserManager();
            if(_user_manager == NULL)
            {
                LOG(FATAL,"_user_manager init failed!") << std::endl;
                exit(7);
            }
            LOG(INFO,"UserManager init success!!!") << std::endl;
        }
        void Start()
        {
            //1、创建UDP生产者消费者线程（将udp缓冲区的信息存放到数据池，将数据池的信息发送给客户）
            for(size_t i = 0; i < THREAD_NUM; ++i)
            {
                pthread_t tid;
                int ret = pthread_create(&tid,NULL,ProductMsgStart,(void*)this);
                if(ret < 0)
                {
                    LOG(FATAL,"ProductMsgStart create failed!") << std::endl;
                    exit(11);
                }
                ret = pthread_create(&tid,NULL,ConsumeMsgStart,(void*)this);
                if(ret < 0)
                {
                    LOG(FATAL,"ConsumeMsgStart create failed!") << std::endl;
                    exit(12);
                }
            }
            LOG(INFO,"Product-Consume threads start!!!") << std::endl;
            //2、创建TCP登陆/注册的线程
            //主线程用来监听，然后accept新的socket，新线程用新的socket去服务一个对象
            while(1)
            {
                //2.1、accept 一个客户
                struct sockaddr_in cli_tcp_addr;
                socklen_t addr_len = sizeof(cli_tcp_addr);
                int new_sock = accept(_tcp_sock,(struct sockaddr*)&cli_tcp_addr,&addr_len);
                if(new_sock < 0)
                {
                    LOG(ERROR,"accept new_sock failed!") << std::endl;
                    continue;
                }
                //2.2、把new_sock和ChatServer的this指针封装了传入一个线程里
                //为什么要封装起来：因为pthread只能传入一个参数
                NewsockAndChatserver* nac = new NewsockAndChatserver(new_sock,this);
                if(nac == NULL)
                {
                    LOG(ERROR,"new NewsockAndChatserver failed!") << std::endl;
                    continue;
                }
                //3、将这个封装的参数传入一个线程，让这个线程来处理登陆或者注册
                pthread_t tid;
                int ret = pthread_create(&tid,NULL,LoginRegisterStart,(void*)nac);
                if(ret < 0)
                {
                    LOG(ERROR,"Create LoginRegisterStart thread failed!") << std::endl;
                    continue;
                }
                LOG(INFO,"Create LoginRegisterStart thread success!!!") << std::endl;
            }
        }
        static void* ProductMsgStart(void* arg)
        {
           //注意要分离线程，否则会资源泄漏，毕竟我们也不需要去关注线程返回状态
           pthread_detach(pthread_self());
           ChatServer* cs = (ChatServer*)arg; 
           while(1)
           {
               cs->RecvMsg();
           }
           return NULL;
        }
        static void* ConsumeMsgStart(void* arg)
        {
           //注意要分离线程，否则会资源泄漏，毕竟我们也不需要去关注线程返回状态
           pthread_detach(pthread_self());
           ChatServer* cs = (ChatServer*)arg; 
           while(1)
           {
               cs->BroadcastMsg();
           }
           return NULL;
        }
        static void* LoginRegisterStart(void* arg)
        {
           //注意要分离线程，否则会资源泄漏，毕竟我们也不需要去关注线程返回状态
            pthread_detach(pthread_self());
            //1、拿到封装的两个数据
            NewsockAndChatserver* nac = (NewsockAndChatserver*)arg;
            ChatServer* cs = nac->_cs;
            int new_sock = nac->_new_sock;
            //2、去new_sock的缓冲区去取一个char大小的请求
            enum RegisterAndLogin request_type;
            int recv_size = recv(new_sock,&request_type,sizeof(request_type),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"recv request_type error!") << std::endl;
                return NULL;
            }
            if(recv_size == 0)
            {
                LOG(ERROR,"client tcp shutdown!") << std::endl;
                return NULL;
            }
            //ReplyIngo里要返回给客户的本次执行状态和用户id
            enum ReplyStatus response_status = REQUEST_ERROR;
            uint32_t response_id = -1;
            //3、通过收到的请求类型来判断是 注册 还是 登陆
            switch(request_type)
            {
                case REGISTER:
                    response_status = cs->DealRegister(new_sock,&response_id);
                    break;
                case LONGIN:
                    response_status = cs->DealLogin(new_sock);
                    break;
                default:
                    response_status = REQUEST_ERROR; 
                    break;
            }
            //4、组织要回给客户的ReplyInfo
            ReplyInfo response;
            response._status = response_status;
            response._user_id = response_id;
            //5、发送给客户端这次的处理情况
            int send_size = send(new_sock,&response,sizeof(response),0);
            if(send_size < 0)
            {
                LOG(ERROR,"send ReplyInfo failed!") << std::endl;
                return NULL;
            }
            LOG(INFO,"Send ReplyInfo success!!!") << std::endl;
            //6、关闭new_sock套接字
            close(new_sock);
            delete nac;
            return NULL;
        }
    private:
        //处理注册
        ReplyStatus DealRegister(int new_sock,uint32_t* user_id)
        {
            //从newsock的接受窗口接受RegisterInfo大小的注册数据
            RegisterInfo ri;
            int recv_size = recv(new_sock,&ri,sizeof(ri),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"recv RegisterInfo failed!") << std::endl;
                return REGISTER_FAILED;
            }
            if(recv_size == 0)
            {
                LOG(ERROR,"client tcp shutdown!") << std::endl;
                return REGISTER_FAILED;
            }
            //将注册信息交给user_manager来处理注册
            bool ret =_user_manager->Register(ri._nick_name,ri._school,ri._password,user_id);
            if(ret == false)
            {
                return REGISTER_FAILED;
            }
            return REGISTER_SUCCESS; 
        }
        //处理登陆
        ReplyStatus DealLogin(int new_sock)
        {
            //在new_sock的接收窗口接受LoginInfo大小的登陆数据
            LoginInfo li;
            int recv_size = recv(new_sock,&li,sizeof(li),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"recv LoginInfo failed!") << std::endl;
                return LOGIN_FAILED;
            }
            if(recv_size == 0)
            {
                LOG(ERROR,"client tcp shutdown!") << std::endl;
                return LOGIN_FAILED;
            }
            //把接受到的LoginInfo里的账号和密码交给user_manager校验
            bool ret = _user_manager->Login(li._user_id,li._password);
            if(ret == false)
            {
                LOG(ERROR,"user_id or password is not correct!") << std::endl;
                return LOGIN_FAILED;
            }
            return LOGIN_SUCCESS;
        }

        //从udp接受缓冲区接受一条消息，并解析其合法性和地址信息
        void RecvMsg()
        {
            //1、从udp缓冲区中拿信息数据
            char buf[10240] = {'\0'};
            struct sockaddr_in cli_udp_addr;
            socklen_t addr_len = sizeof(cli_udp_addr);
            int recv_size = recvfrom(_udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&cli_udp_addr,&addr_len);
            if(recv_size < 0)
            {
                LOG(ERROR,"RecvMsg failed!") << std::endl;
                return;
            }
            //2、反序列化该信息 
            std::string recv_msg;
            recv_msg.assign(buf,recv_size);
            FormatMessage fmsg; 
            fmsg.Deserialize(recv_msg);
            //3、从该信息中提取用户信息，看是否是合法用户
            //  3.1：如果在用户信息列表里没有找到，则这条信息不放入数据池
            //  3.2：如果在在线用户列表里没有找到，则这条信息也不放入数据池
            //  3.3：如果是登陆的信息(tcp登录完成后会立即发送udp，因为用户聊天是用的udp)，则也不放入数据池，而是把用户登陆
            bool ret = _user_manager->IsLogin(fmsg._user_id,cli_udp_addr,addr_len);
            if(ret == true)
            {
                _msg_pool->PushMsgToPool(recv_msg);
                LOG(INFO,"PushMsgToPool success!!! ") << fmsg._msg  <<std::endl;
            }
        }
        //发送消息给所有在线用户
        void BroadcastMsg()
        {
            //1、获取所有在线用户列表
            std::vector<UserInfo> online_list = _user_manager->GetOnlinelist();
            //2、从数据池中取数据
            std::string msg;
            _msg_pool->PopMsgFromPool(&msg);
            //3、一个个发送
            for(const auto& user:online_list)
            {
                SendMsg(msg,user._addr,user._addr_len);
            }
        }
        //发送消息给一个客户
        void SendMsg(const std::string& msg,const struct sockaddr_in& cli_udp_addr,const socklen_t& addr_len)
        {
            int send_size = sendto(_udp_sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&cli_udp_addr,addr_len);
            if(send_size < 0)
            {
                LOG(ERROR,"SendMsg failed!") << std::endl;
                return;
            }
            LOG(INFO,"SendMsg success!!!") << msg << std::endl;
        }
};

