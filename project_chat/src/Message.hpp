#pragma once
#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>

class Message
{
    public:
        std::string _nick_name;
        std::string _school;
        std::string _msg;
        uint32_t _user_id;
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
        void Serialize(std::string* message)
        {
            Json::Value val;
            val["_nick_name"] = _nick_name;
            val["_school"] = _school;
            val["_msg"] = _msg;
            val["_user_id"] = _user_id;

            Json::FastWriter writer;
            *message = writer.write(val);
        }
        uint32_t& GetUserId()
        {
            return _user_id;
        }
};
