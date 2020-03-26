#include<iostream>
#include<unordered_map>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string>

#define ONLINE 1
#define OFFLINE -1

class UserInfo
{
private:
    std::string _nick_name;
    std::string _school;
    uint64_t _usr_id;
    std::string _password;
    //保存的是：发消息时候的udp客户端的信息
    struct sockaddr_in _cli_addr; 
    socklen_t _addr_len;
    //当前账号的状态
    int _usr_status;
public:
    UserInfo(std::string& nick_name, std::string school, uint64_t usr_id, std::string password)
    {
        _nick_name = nick_name;
        _school = school;
        _usr_id = usr_id;
        _password = password;
        memset(&_cli_addr,'0',sizeof(struct sockaddr_in));
        _addr_len = -1;
        _usr_status = OFFLINE;
    }
};

class UserManager
{
private:
    //用来保存用户所有的信息
    std::unordered_map<uint64_t,UserInfo> _user_map;   
    pthread_mutex_t _mtx;
public:
    UserManager()
    {
        _user_map.clear();
        pthread_mutex_init(&_mtx,NULL);
    }
    ~UserManager()
    {
        pthread_mutex_destroy(&_mtx);
    }
    void Register()
    {

    }
    void Login()
    {

    }
    void LoginOut()
    {

    }
};
