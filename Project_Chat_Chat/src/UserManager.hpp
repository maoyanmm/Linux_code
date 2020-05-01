#pragma once
#include<string>
#include<cstring>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<arpa/inet.h>

#include"Log.hpp"

//该用户信息管理模块是用来将注册了的、登陆了的用户信息进行管理（登陆、注册）

//表示当前用户的状态
enum UserStatus
{
    OFFLINE = 0,//下线
    ONLINE,//上线
    WILL_BE_ONLINE
};

class UserInfo
{
    public:
        //基本信息
        std::string _nick_name;  
        std::string _school;
        std::string _password;
        uint32_t _user_id;
        //聊天时候用的udp信息
        struct sockaddr_in _addr;
        socklen_t _addr_len;
        UserStatus _status;
    public:
        UserInfo(const std::string& nick_name,const std::string& school,const std::string& password,uint32_t user_id)
            :_nick_name(nick_name),_school(school),_password(password),_user_id(user_id)
        {
            //默认是下线状态
            _status = OFFLINE;
            memset(&_addr,0,sizeof(_addr));
            _addr_len = -1;
        }
};

class UserManager
{
    private:
        pthread_mutex_t _lock;
        //所有用户信息列表
        std::unordered_map<uint32_t,UserInfo> _user_info_list;                
        //在线用户列表
        std::unordered_set<UserInfo> _online_list;
        //预分配的用户id
        uint32_t _prepare_user_id;
    public:
        UserManager()
        {
            _prepare_user_id = 1;
            pthread_mutex_init(&_lock,NULL);
        }
        ~UserManager()
        {
            pthread_mutex_destroy(&_lock);
        }
        bool Register(const std::string nick_name,const std::string& school, const std::string& password,uint32_t* id)
        {
            //如果输入的注册信息为空，则注册失败
            if(nick_name.size() == 0 || school.size() == 0 || password.size() == 0)
            {
                return false;
            }
            pthread_mutex_lock(&_lock);
            //1、创建一个用户信息对象，并将注册的信息填入
            UserInfo user_info(nick_name,school,password,_prepare_user_id);
            user_info._status = OFFLINE;
            //2、将这个用户信息插入到用户信息列表里
            _user_info_list.insert(std::make_pair(_prepare_user_id,user_info));
            //3、返回注册得到的新ID
            *id = _prepare_user_id;
            //4、将预分配的id++
            ++_prepare_user_id;
            LOG(INFO,"Register a user success") << std::endl;
            pthread_mutex_unlock(&_lock);
        }
        bool Login(uint32_t user_id,const std::string& password)
        {
            pthread_mutex_lock(&_lock);
            //查看该id是否存在
            auto it = _user_info_list.find(user_id);
            if(it == _user_info_list.end())
            {
                LOG(ERROR,"UserId is not exists") << std::endl;
                pthread_mutex_unlock(&_lock);
                return false;
            }
            //校验密码
            if(it->second._password != password)
            {
                LOG(ERROR,"Password or UserId is not correct") << std::endl;
                pthread_mutex_unlock(&_lock);
                return false;
            }
            //密码正确，把用户状态更新
            it->second._status = WILL_BE_ONLINE;
            pthread_mutex_unlock(&_lock);
            return true;
        }
        bool IsLogin(const uint32_t& user_id,const struct sockaddr_in& cli_addr,const socklen_t& addr_len)
        {
            if(sizeof(cli_addr) <= 0 || addr_len <= 0)
            {
                return false;
            }
            pthread_mutex_lock(&_lock);
            //1、先查找是否存在用户
            auto it = _user_info_list.find(user_id);
            if(it == _user_info_list.end())
            {
                pthread_mutex_unlock(&_lock);
                return false;
            }
            //2、判断用户的状态
            if(it->second._status == OFFLINE)
            {
                pthread_mutex_unlock(&_lock);
                return false;
            }
            //如果是即将上线的，也就是说这是一条登陆的信息，则更新用户状态,并把它放入在线列表里
            if(it->second._status == WILL_BE_ONLINE)
            {
                it->second._status = ONLINE;
                it->second._addr = cli_addr;
                it->second._addr_len = addr_len;
                _online_list.insert(it->second);
                pthread_mutex_unlock(&_lock);
                return false;
            }
            pthread_mutex_unlock(&_lock);
            return true; 
        }
        std::unordered_set<UserInfo>& GetOnlinelist()
        {
            return _online_list;
        }
};
