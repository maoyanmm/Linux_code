#pragma once
#include<iostream>
#include<jsoncpp/json/json.h>
#include<string>

//该模块客户端和服务端都使用，是用来给收到的数据进行反序列化，将要发送的数据进行反序列化

class FormatMessage
{
    public:
        uint32_t _user_id;
        std::string _nick_name;
        std::string _birthday;
        //需要发送的信息内容
        std::string _msg;
    public:
        //反序列化：将发送来的msg进行格式组织，存到成员变量里
        void Deserialize(const std::string& msg)
        {
            Json::Reader reader;
            Json::Value value;
            reader.parse(msg,value,false);
            
            _user_id = value["_user_id"].asInt();
            _nick_name = value["_nick_name"].asString();
            _birthday = value["_birthday"].asString();
            _msg = value["_msg"].asString();
        }
        //序列化：将成员变量序列化成字符串
        void Serialize(std::string* msg)
        {
            Json::Value value;
            value["_user_id"] = _user_id;
            value["_nick_name"] = _nick_name;
            value["_birthday"] = _birthday;
            value["_msg"] = _msg;

            Json::FastWriter writer;
            *msg = writer.write(value);
        }
};
