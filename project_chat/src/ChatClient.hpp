#pragma once
#include<stdio.h>
#include<iostream>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>

#include"LogSvr.hpp"

#define UDP_PORT 4418
#define TCP_PORT 4419

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
    public:
        ChatClient(std::string svr_ip = "127.0.0.1")
        {
            _udp_sock = -1;
            _udp_port = UDP_PORT;
            _tcp_sock = -1;
            _tcp_port = TCP_PORT;
            _svr_ip = svr_ip;
        }
        ~ChatClient()
        {
            if(_udp_sock > 0)
            {
                close(_udp_sock);
            }
        }
        void Init()
        {
            _udp_sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
            if(_udp_sock < 0)
            {
                LOG(FATAL,"Init _udp_sock failed!") << std::endl;
                exit(1);
            }
            _tcp_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            if(_tcp_sock < 0)
            {
                LOG(FATAL,"Init _tcp_sock failed!") << std::endl;
                exit(2);
            }
            if(ConnectServer() == false)
            {
                exit(3);
            }

        }
        bool Login();
        bool Register();
        bool SendMsg();
        bool ReceiveMsg();
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

