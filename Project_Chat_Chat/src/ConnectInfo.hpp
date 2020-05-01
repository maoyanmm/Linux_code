#pragma once
#include<string>

//该模块用来给TCP收发数据来规定格式的，里面包含三个收发数据的结构体

//返回给客户的状态信息
enum ReplyStatus
{
    REGISTER_SUCCESS = 0,
    REGISTER_FAILED,
    LOGIN_SUCCESS,
    LOGIN_FAILED
};
//客户发送来的注册信息
struct RegisterInfo
{
    char _nick_name[15];
    char _school[20];
    char _password[15];
};
//客户发送来的登录信息
struct LoginInfo
{
    uint32_t _user_id;
    char _password[15];
};
//服务端返回给客户端的信息
struct ReplyInfo
{
    uint32_t _user_id;
    ReplyStatus _status;
};
