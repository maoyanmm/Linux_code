#pragma once
#include<iostream>
#include<string>

#define REGISTER 1
#define LOGIN 2
#define LOGINOUT 3

enum UserStatus
{
    REGIST_FAILED = 0,//注册失败
    REGIST_SUCCESS,//注册成功
    LOGIN_FAILED,//注册失败
    LOGIN_SUCCESS//注册成功
};

//注册信息
struct RegestInfo
{
    char _nick_name[15];
    char _school[20];
    char _password[15];
};
//注册后的应答信息
struct ReplyInfo
{
    //当前注册状态
    //OFFLINE/REGISTERED/LOGINED/ONLINE
    int _status;
    uint64_t _user_id;
};
//登录信息
struct LoginInfo
{
    uint64_t _user_id;
    char _password[20];
};
//这个类就是封装了new_sock和ChatServer类，为了方便用到这两个变量
class LoginConnect
{
    private:
        int _tcp_sock;
        //为了保存ChatServer的指针
        void* _server;
    public:
        LoginConnect(int sock,void* server)
            :_tcp_sock(sock),_server(server)
        {

        }
        int GetTcpSock()
        {
            return _tcp_sock;
        }
        void* GetServer()
        {
            return _server;
        }
};
