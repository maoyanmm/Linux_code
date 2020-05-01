#pragma once
#include<iostream>
#include<stdio.h>
#include<cstring>

//日志模块是给所有其他模块打印当前代码的详细信息用的

const char* LevelStr[] = {
    "INFO","WARNING","ERROR","FATAL"
}; 

enum Level
{
    INFO = 0,
    WARNING,
    ERROR,
    FATAL
};

void GetTime(std::string* time_stamp)
{
    //拿到现在时间的时间戳
    time_t now_time;
    time(&now_time);
    //格式化时间戳
    struct tm* st = localtime(&now_time);
    char format_time[23] = {'\0'};
    snprintf(format_time,sizeof(format_time)-1,"%04d-%02d-%02d %02d:%02d:%02d",st->tm_year+1900,st->tm_mon+1,st->tm_mday,st->tm_hour,st->tm_min,st->tm_sec);
    time_stamp->assign(format_time,strlen(format_time));
}

std::ostream& Log(Level level,const std::string& msg,const char* file,int line)
{
    std::string level_info = LevelStr[level];
    std::string time_now;
    GetTime(&time_now);
    std::cout << "[" << time_now << " " << level_info << " " << file << ":" << line << "]" << msg;
    return std::cout;
}

#define LOG(level,msg) Log(level,msg,__FILE__,__LINE__)
