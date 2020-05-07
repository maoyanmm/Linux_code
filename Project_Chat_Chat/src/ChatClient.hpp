#pragma once
#include<iostream>
#include<unistd.h>
#include<arpa/inet.h>
#include<unordered_set>

#include"Log.hpp"
#include"FormatMessage.hpp"
#include"ConnectInfo.hpp"

#define SVR_TCP_PORT 4418
#define SVR_UDP_PORT 4419
#define SVR_IP "192.168.132.128"
#define MAX_MESSAGE_SIZE 1024

class MyOnlineInfo
{
    public:
        std::string _nick_name; 
        std::string _birthday;
        uint32_t _user_id;
};

class ChatClient
{
    private:
        //服务端的用来发消息的udp
        uint16_t _svr_udp_port;
        //服务端的用来登陆/注册的tcp
        uint16_t _svr_tcp_port;
        //服务端的ip地址
        std::string _svr_ip;

        //客户端用来登陆/注册的tcp_sock
        int _cli_tcp_sock;
        //客户端用来发消息的udp_sock
        int _cli_udp_sock;
        //在线用户列表
        std::unordered_set<std::string> _online_list;
        //当前登陆的用户信息
        MyOnlineInfo _me;
    public:
        ChatClient(const std::string& svr_ip = SVR_IP)
        {
            _svr_udp_port = SVR_TCP_PORT;
            _svr_tcp_port = SVR_UDP_PORT;
            _svr_ip = svr_ip;
            _cli_tcp_sock = -1;
            _cli_udp_sock = -1;
            InitUdp();
            //这里没有去初始化TCP，因为虽然登陆和注册都要用到TCP，但是一个客户的请求可能不一样，所以还是放在了登陆和注册函数里去初始化
        }
        ~ChatClient()
        {
            if(_cli_udp_sock != -1)
            {
                close(_cli_udp_sock);
            }
            //这里没有去关闭_cli_tcp_sock，因为在注册和登陆完后都会关闭掉
        }
    public:
        bool Register()
        {
            //如果连接失败会自动退出进程
            InitAndConnectTcp();
            //1、先发送ConnectInfo里的枚举类型的注册请求
            enum RegisterAndLogin request = REGISTER;
            int send_size = send(_cli_tcp_sock,&request,sizeof(request),0);
            if(send_size < 0)
            {
                LOG(ERROR,"Send Register request failed!") << std::endl;
                return false;
            }
            //2、发送注册请求后，填写注册的基本信息RegisterInfo
            RegisterInfo register_info; 
            std::cout << "Please enter your nick_name: " << std::endl;
            std::cin >> register_info._nick_name;
            std::cout << "Please enter your birthday: " << std::endl;
            std::cin >> register_info._birthday;
            while(1)
            {
                char password1[15] = {0}; 
                char password2[15] = {0}; 
                std::cout << "Please enter your password: " << std::endl;
                std::cin >> password1;
                std::cout << "Please enter your password again: " << std::endl;
                std::cin >> password2;
                //如果两次输入的密码不同则重新输入
                if(strcmp(password1,password2) == 0)
                {
                    //密码相同则把输入的密码打包
                    strcpy(register_info._password,password1);
                    break;
                }
                else
                {
                    std::cout << "Twice password were not same! Please enter again!" << std::endl;
                }
            }
            //3、发送注册的信息 
            send_size = send(_cli_tcp_sock,&register_info,sizeof(register_info),0);
            if(send_size < 0)
            {
                LOG(ERROR,"Send RegisterInfo failed!") << std::endl;
                return false;
            }
            //4、查看服务端回过来的注册情况
            ReplyInfo response; 
            int recv_size = recv(_cli_tcp_sock,&response,sizeof(response),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"Recv Register ReplyInfo failed!") << std::endl;
                return false;
            }
            if(recv_size == 0)
            {
                LOG(ERROR,"Server tcp shutdown!") << std::endl;
                return false;
            }
            //  如果请求注册时候填的信息不对或为空则错误
            if(response._status == REGISTER_FAILED || response._status == REQUEST_ERROR)
            {
                LOG(ERROR,"Request error,please check register info!") << std::endl;
                return false;
            }
            std::cout << "Register success! Your new user_id : " << response._user_id << std::endl;
            close(_cli_tcp_sock);
            return true;
        }
        bool Login()
        {
            InitAndConnectTcp();
            //1、给服务端发送登陆的请求
            enum RegisterAndLogin request = LOGIN;
            int send_size = send(_cli_tcp_sock,&request,sizeof(request),0); 
            if(send_size < 0)
            {
                LOG(ERROR,"Send Login request failed!") << std::endl;
                return false;
            }
            //2、组织LoginInfo里的id和密码
            LoginInfo li;
            std::cout << "Please enter your user_id" << std::endl;
            std::cin >> li._user_id;
            std::cout << "Please enter your password" << std::endl;
            std::cin >> li._password;
            //3、发送LoginInfo
            send_size = send(_cli_tcp_sock,&li,sizeof(li),0);
            if(send_size < 0)
            {
                LOG(ERROR,"Send LoginInfo failed!") << std::endl;
                return false;
            }
            //4、查看服务端返回的登陆状态
            ReplyInfo response;
            int recv_size = recv(_cli_tcp_sock,&response,sizeof(response),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"Recv Login ReplyInfo failed!") << std::endl;
                return false;
            }
            if(recv_size == 0)
            {
                LOG(ERROR,"Server tcp shutdown!") << std::endl;
                return false;
            }
            if(response._status != LOGIN_SUCCESS)
            {
                LOG(ERROR,"Login failed! Please check your user_id and password!") << std::endl;
                return false;
            }
            //5、登陆成功后把当前登陆的用户信息存起来
            _me._nick_name = response._nick_name;
            _me._birthday = response._birthday;
            _me._user_id = response._user_id;
            //6、发送udp数据：为了让服务端知道自己的UDP端口(因为聊天用的是UDP)
            //这一步做完才算是真正的登陆了,这一步只需要发送一个序列化消息即可，msg可以是空
            //tcp登陆消息是为了验证密码，udp登陆消息是为了告诉服务端自己的udp信息
            SendMsg("");
            std::cout << "Login success!!!" << std::endl;
            close(_cli_tcp_sock); 
            return true;
        }
        bool SendMsg(const std::string& msg)
        {
            //1、组织好要发送的信息格式
            //如果发送的消息超过了最大的size，则截断
            FormatMessage fmsg;
            if(msg.size() > MAX_MESSAGE_SIZE)
            {
                fmsg._msg = msg.substr(0,MAX_MESSAGE_SIZE);    
            }
            else
            {
                fmsg._msg = msg;
            }
            fmsg._msg = msg;
            fmsg._nick_name = _me._nick_name;
            fmsg._birthday = _me._birthday;
            fmsg._user_id = _me._user_id;
            std::string send_msg;
            fmsg.Serialize(&send_msg);

            //2、组织好服务端的地址信息
            struct sockaddr_in svr_addr;
            svr_addr.sin_family = AF_INET;
            svr_addr.sin_port = htons(_svr_udp_port);
            svr_addr.sin_addr.s_addr = inet_addr(_svr_ip.c_str());

            //3、发送
            int send_size = sendto(_cli_udp_sock,send_msg.c_str(),send_msg.size(),0,(struct sockaddr*)&svr_addr,sizeof(svr_addr));
            if(send_size < 0)
            {
                LOG(ERROR,"SendMsg failed!") << std::endl;
                return false;
            }
            return true;
        }
        bool RecvMsg(std::string* msg)
        {
            //从UDP缓冲区拿数据
            char buf[MAX_MESSAGE_SIZE] = {'\0'};
            memset(buf,'\0',MAX_MESSAGE_SIZE);
            struct sockaddr_in svr_addr;
            socklen_t addr_len = sizeof(svr_addr);
            int recv_size = recvfrom(_cli_udp_sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&svr_addr,&addr_len);
            if(recv_size < 0)
            {
                LOG(ERROR,"RecvMsg failed!") << std::endl;
                return false;
            }
            msg->assign(buf,recv_size);
            return true;
        }
        void PushOnlineUser(const std::string& user_info)
        {
            auto it = _online_list.find(user_info);
            if(it == _online_list.end())
            {
                _online_list.insert(user_info);
            }
        }
        std::unordered_set<std::string>& GetOnlineUser()
        {
            return _online_list;
        }

    private:
        void InitUdp()
        {
            _cli_udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
            if(_cli_udp_sock < 0)
            {
                LOG(FATAL,"InitUdp failed!") << std::endl;
                exit(1);
            }
        }
        void InitAndConnectTcp()
        {
            //1、初始化套接字
            _cli_tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            if(_cli_tcp_sock < 0)
            {
                LOG(FATAL,"InitTcp failed!") << std::endl;
                exit(2);
            }
            //2、连接服务端
            struct sockaddr_in svr_addr;
            svr_addr.sin_family = AF_INET;
            svr_addr.sin_port = htons(_svr_tcp_port);
            svr_addr.sin_addr.s_addr = inet_addr(_svr_ip.c_str());

            int ret = connect(_cli_tcp_sock,(struct sockaddr*)&svr_addr,sizeof(svr_addr));
            if(ret < 0)
            {
                LOG(FATAL,"Connect SvrTcp failed!") << std::endl;
                exit(3);
            }
        }

};
