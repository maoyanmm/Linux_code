#pragma once
#include<iostream>
#include<sys/socket.h>
#include<sys/stat.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<cstring>
#include<string>
#include<unordered_map>
#include<vector>

#include"LogSvr.hpp"

enum UserStatus
{
    OFFLINE = 0,
    REGISTERED,
    LOGINED,//表示登陆了还没发消息
    ONLINE//表示登陆了而且发过消息
};

//单个用户的信息类
class UserInfo
{
    private:
        //基本信息
        std::string _nick_name;
        std::string _school;
        uint64_t _user_id;
        std::string _password;
        //网络信息
        struct sockaddr_in _addr;
        socklen_t _addr_len;
        int _user_status;
    public:
        UserInfo(const std::string& nick_name = " ",const std::string& school = " ",uint64_t user_id = 0,const std::string& password = " ")
            :_nick_name(nick_name),_school(school),_user_id(user_id),_password(password)
        {
            memset(&_addr,'0',sizeof(_addr));
            _addr_len = -1;            
            _user_status = OFFLINE;
        }
        void SetUserStatus(int user_status)
        {
            _user_status = user_status;
        }
        int& GetUserStatus()
        {
            return _user_status;
        }
        std::string& GetPassword()
        {
            return _password;
        }
        void SetCliAddrInfo(const struct sockaddr_in& cli_addr_info,const socklen_t& cli_addr_len)
        {
            memcpy(&_addr,&cli_addr_info,sizeof(cli_addr_info));
            _addr_len = cli_addr_len;
        }
        struct sockaddr_in& GetCliAddr()
        {
            return _addr; 
        }
        socklen_t& GetCliAddrLen()
        {
            return _addr_len;
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
        //预分配的用户id
        uint64_t _prepare_user_id;
    public:
        UserManager()
        {
            _user_map.clear();
            _online_list.clear();
            pthread_mutex_init(&_mtx,NULL);
            _prepare_user_id = 0;
        }
        ~UserManager()
        {
            pthread_mutex_destroy(&_mtx);
        }
        int Register(const std::string& nick_name,const std::string& school,const std::string& password,uint64_t* user_id)
        {
            if(nick_name.size() == 0 || school.size() == 0 || password.size() == 0)
            {
                return -1;
            }

            pthread_mutex_lock(&_mtx);
            //1、根据客户传来的客户信息创建一个用户信息的对象
            UserInfo user_info(nick_name,school,_prepare_user_id,password);
            //2、更新该用户状态为已注册
            user_info.SetUserStatus(REGISTERED); 
            //3、将这个用户信息插入到列表里
            _user_map.insert(std::make_pair(_prepare_user_id,user_info));
            //4、将生成的UserId返还给客户
            *user_id = _prepare_user_id;
            //5、将预分配的UserId++
            ++_prepare_user_id;
            pthread_mutex_unlock(&_mtx);

            return 0;
        }
        int Login(const uint64_t& user_id,const std::string& password)
        {
            if(password.size() < 0)
            {
                return -1;
            }
            //1、在map里查找是否存在user_id
            //加锁是因为后面会改变用户状态
            pthread_mutex_lock(&_mtx);
            int login_status = -1;
            auto it = _user_map.find(user_id);
            //如果没找到，则报错
            if(it == _user_map.end())
            {
                login_status = -1;
            }
            //如果找到了，则验证密码
            else
            {
                //如果密码正确
                if(it->second.GetPassword() == password)
                {
                    it->second.GetUserStatus() = LOGINED;
                    login_status = 0;
                }
                //如果密码不正确
                else
                {
                    login_status = -1;
                }
            }
            pthread_mutex_unlock(&_mtx);
            return login_status;
        }
        int LoginOut();
        bool IsLogin(uint64_t user_id,const struct sockaddr_in& cli_addr,const socklen_t& addr_len)
        {
            if(sizeof(cli_addr) < 0 || addr_len < 0)
            {
                return false;
            }
            //1、检测用户是否存在
            auto it = _user_map.find(user_id); 
            if(it == _user_map.end())
            {
                LOG(ERROR,"User not exist") << std::endl;
                return false;
            }
            //2、判断当前用户状态
            //如果是下线的状态则false
            pthread_mutex_lock(&_mtx);
            if(it->second.GetUserStatus() == OFFLINE)
            {
                LOG(ERROR,"User status error") << std::endl;
                pthread_mutex_unlock(&_mtx);
                return false;
            }
            //如果不是第一次发送消息，就不需要再次把客户的IP和PORT存起来
            if(it->second.GetUserStatus() == ONLINE)
            {
                pthread_mutex_unlock(&_mtx);
                return true;
            }
            //如果是第一次发送消息，则需要把客户的IP和PORT存起来
            if(it->second.GetUserStatus() == LOGINED)
            {
                it->second.SetCliAddrInfo(cli_addr,addr_len);
                it->second.SetUserStatus(ONLINE);
                _online_list.push_back(it->second);
                std::cout << "在线列表插入了新的客户：" << it->first << std::endl;
            }
            pthread_mutex_unlock(&_mtx);
            return true;
        }
        void GetOnlineUser(std::vector<UserInfo>* list)
        {
            *list = _online_list;
        }

};
