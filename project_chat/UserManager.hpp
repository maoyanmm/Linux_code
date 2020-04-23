#pragma once
#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<cstring>
#include<string>
#include<unordered_map>
#include<vector>

#define OFFLINE 0
#define REGISTER 1
#define LOGINED 2
#define ONLINE 3

//单个用户的信息类
class UserInfo
{
    private:
        std::string _nick_name;
        std::string _school;
        uint64_t _user_id;
        std::string _password;
        struct sockaddr_in _addr;
        socklen_t _addr_len;
        int _user_status;
    public:
        UserInfo(std::string& nick_name,std::string& school,uint64_t user_id,std::string& password)
            :_nick_name(nick_name),_school(school),_user_id(user_id),_password(password)
        {
            memset(&_addr,'0',sizeof(_addr));
            _addr_len = -1;            
            _user_status = OFFLINE;
        }
};
//管理所有用户的信息
class UserManager
{
    private:
        pthread_mutex_t _mtx;
        //保存所有注册了的用户的信息
        std::unordered_map<uint64_t,UserInfo> _user_map;
        //保存所有在线的用户的信息
        std::vector<UserInfo> _online_list;
    public:
        UserManager()
        {
            _user_map.clear();
            _online_list.clear();
            pthread_mutex_init(&_mtx,NULL);
        }
        ~UserManager()
        {
            pthread_mutex_destroy(&_mtx);
        }
        int Register()
        {

        }
        int Login();
        int LoginOut();
};
