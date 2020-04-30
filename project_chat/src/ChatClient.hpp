#pragma once
#include<stdio.h>
#include<iostream>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
#include"jsoncpp/json/json.h"
#include<unordered_set>

#include"LogSvr.hpp"
#include"ConnectInfo.hpp"
#include"UserManager.hpp"

#define SVR_UDP_PORT 4418
#define SVR_TCP_PORT 4419
struct Myself
{
    std::string _nick_name;
    std::string _school;
    std::string _password;
    uint32_t _user_id;
};

class ChatClient
{
    private:
        //UDP用来发送对话消息的
        int _udp_sock;
        int _udp_port;
        //TCP用来发送 登陆/注册信息
        int _tcp_sock;
        int _tcp_port;
        //服务端的IP
        std::string _svr_ip;
        Myself _me;
        std::unordered_set<std::string> _online_list;
    public:
        ChatClient(std::string svr_ip = "192.168.132.128")
        {
            _udp_sock = -1;
            _udp_port = SVR_UDP_PORT;
            _tcp_sock = -1;
            _tcp_port = SVR_TCP_PORT;
            _svr_ip = svr_ip;
            InitUDP();
        }
        ~ChatClient()
        {
            if(_udp_sock > 0)
            {
                close(_udp_sock);
            }
        }
        Myself& GetMyInfo()
        {
            return _me;
        }
        void InitUDP()
        {
            _udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
            if(_udp_sock < 0)
            {
                LOG(FATAL,"Init _udp_sock failed!") << std::endl;
                exit(1);
            }
        }
        void InitTCP()
        {
            _tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            if(_tcp_sock < 0)
            {
                LOG(FATAL,"Init _tcp_sock failed!") << std::endl;
                exit(2);
            }
        }
        bool Login()
        {
            //连接服务端的TCP
            InitTCP();
            if(!ConnectServer())
            {
                LOG(FATAL,"ConnectServer failed!") << std::endl;
                return false;
            }
            //1、先发送登陆请求
            char type = LOGIN;
            ssize_t send_size = send(_tcp_sock,&type,1,0);
            if(send_size < 0)
            {
                LOG(ERROR,"Send Login type failed!") << std::endl;
                return false;
            }
            //2、输入账号和密码
            struct LoginInfo li;
            std::cout << "Please enter your UserId：" << std::endl;
            std::cin >> li._user_id;
            std::cout << "Please enter your Password：" << std::endl;
            std::cin >> li._password;
            send(_tcp_sock,&li,sizeof(li),0);
            //3、查看是否登陆成功
            struct ReplyInfo response;
            ssize_t recv_size = recv(_tcp_sock,&response,sizeof(response),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"Receive Login reply failed!") << std::endl;
                return false;
            }
            else if(recv_size == 0)
            {
                LOG(ERROR,"Server shutdown!") << std::endl;
                return false;
            }
            if(response._status == LOGIN_FAILED)
            {
                LOG(ERROR,"Login failed!") << std::endl;
                return false;
            }
            LOG(ERROR,"Login success!") << std::endl;
            //关闭和服务端的TCP连接
            close(_tcp_sock);
            return true;
        }
        bool Register()
        {
            //连接服务端的TCP
            InitTCP();
            if(!ConnectServer())
            {
                LOG(FATAL,"ConnectServer failed!") << std::endl;
                return false;
            }
            //1、发送注册请求
            char type = REGISTER;
            ssize_t send_size = send(_tcp_sock,&type,1,0); 
            if(send_size < 0)
            {
                LOG(ERROR,"Send Register type failed!") << std::endl;
                return false;
            }
            //2、发送注册信息
            struct RegestInfo ri;
            std::cout << "Please enter your NickName：" << std::endl;
            std::cin >> ri._nick_name;
            std::cout << "Please enter your School：" << std::endl;
            std::cin >> ri._school;
            //只有两次密码输入的正确才行
            while(1)
            {
                std::string password1;
                std::string password2;
                std::cout << "Please enter your Password：" << std::endl;
                std::cin >> password1;
                std::cout << "Please enter your Password again：" << std::endl;
                std::cin >> password2;
                if(password1 == password2)
                {
                    strcpy(ri._password,password1.c_str());
                    break;
                }
                else
                {
                    std::cout << "Two password were not match! Please enter again!" << std::endl;
                }
            }
            send_size = send(_tcp_sock,&ri,sizeof(ri),0);
            if(send_size < 0)
            {
                LOG(ERROR,"Send Register infomation failed!") << std::endl;
                return false;
            }
            //3、查看注册情况
            struct ReplyInfo response;
            ssize_t recv_size = recv(_tcp_sock,&response,sizeof(response),0);
            if(recv_size < 0)
            {
                LOG(ERROR,"Receive Register reply failed!") << std::endl;
                return false;
            }
            else if(recv_size == 0)
            {
                LOG(ERROR,"Server shutdown!") << std::endl;
                return false;
            }
            if(response._status == REGIST_FAILED)
            {
                LOG(ERROR,"Register failed!") << std::endl;
                return false;
            }
            LOG(INFO,"Register success! Your new UserId = ：") << response._user_id << std::endl;
            //保存信息
            _me._nick_name = ri._nick_name;   
            _me._school = ri._school;
            _me._password = ri._password;
            _me._user_id = response._user_id;
            //关闭和服务端的TCP连接
            close(_tcp_sock);
            return true;
        }
        bool SendMsg(const std::string& msg)
        {
            struct sockaddr_in svr_addr;
            svr_addr.sin_family = AF_INET;
            svr_addr.sin_port = htons(_udp_port);
            svr_addr.sin_addr.s_addr = inet_addr(_svr_ip.c_str());
            ssize_t send_size = sendto(_udp_sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&svr_addr,sizeof(svr_addr));
            if(send_size < 0)
            {
                LOG(ERROR,"SendMsg failed!") << std::endl;
                return false;
            }
            return true;
        }
        bool ReceiveMsg(std::string* msg)
        {
            char buf[MAX_MESSAGE_SIZE] = {0};
            memset(buf,'\0',MAX_MESSAGE_SIZE);
            struct sockaddr_in svr_addr;
            socklen_t addr_len;
            ssize_t recv_size = recvfrom(_udp_sock,buf,MAX_MESSAGE_SIZE-1,0,(struct sockaddr*)&svr_addr,&addr_len);
            if(recv_size < 0)
            {
                LOG(ERROR,"ReceiveMsg failed!") << std::endl;
                return false;
            }
            msg->assign(buf,recv_size);
            return true;
        }
        void PushOnlineUser(std::string& user_info)
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
        bool ConnectServer()
        {
            struct sockaddr_in svr_addr;
            svr_addr.sin_family = AF_INET;
            svr_addr.sin_port = htons(_tcp_port);
            svr_addr.sin_addr.s_addr = inet_addr(_svr_ip.c_str());

            int ret = connect(_tcp_sock,(struct sockaddr*)&svr_addr,sizeof(svr_addr));
            if(ret < 0)
            {
                LOG(ERROR,"ConnectServer failed!") << std::endl;
                return false;
            }
            return true;
        }
};

