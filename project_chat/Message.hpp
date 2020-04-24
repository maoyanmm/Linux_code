#pragma once
#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>

class Message
{
    private:
        std::string _nick_name;
        std::string _school;
        std::string _msg;
        uint64_t _user_id;
    public:
        void Deserialize(std::string message)
        {
            Json::Reader reader;
            Json::Value val;
            reader.parse(message,val,false);
            _nick_name = val["_nick_name"].asString();
            _school = val["_school"].asString();
            _msg = val["_msg"].asString();
            _user_id = val["_user_id"].asInt();
        }
        uint64_t& GetUserId()
        {
            return _user_id;
        }
};
