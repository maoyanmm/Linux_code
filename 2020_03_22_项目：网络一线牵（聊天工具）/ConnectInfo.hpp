#include<iostream>

#define REGISTER 0
#define LOGIN 1
#define LOGINOUT -1

//用户注册要填的信息
struct RegisterInfo
{
    char _nick_name[10];
    char _school[20];
    char _password[15];
};

//服务端反馈给刚注册的用户信息
struct ReplyInfo
{
    int _status;
    uint64_t _usr_id;
};

//登录的时候需要填的信息
struct LoginInfo
{
    uint64_t _usr_id;
    char _password[20];
};

//就是为了存Chatsever的指针 和 注册或者登陆的TCP套接字
class LoginConnect
{
public: 
    LoginConnect(int sock, void* server)
    {
        _sock = sock;
        _server = server;
    }
    int GetTcpSock()
    {
        return _sock;
    }
    void* GetServer()
    {
        return _server;
    }

private:
    int _sock;
    void* _server;
};
