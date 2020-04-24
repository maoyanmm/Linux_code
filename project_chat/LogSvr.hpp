#pragma once
#include<cstring>
#include<string>
#include<cstdio>
#include<cstdlib>
#include<sys/time.h>
#include<iostream>

const char* Level[] = {
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL",
    "DEBUG"
};

enum LogLevel
{
    INFO = 0,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};

class LogTime
{
    public:
        static void GetTime(std::string* time_stamp)
        {
            //返回：年月日 时分秒
            time_t sys_time;
            time(&sys_time);
            struct tm* st = localtime(&sys_time);
            //格式化字符串
            char time_now[23] = {'\0'}; 
            snprintf(time_now,sizeof(time_now)-1,"%04d-%02d-%02d %02d:%02d:%02d",st->tm_year+1900,st->tm_mon+1,st->tm_mday,st->tm_hour,st->tm_min,st->tm_sec);
            time_stamp->assign(time_now,strlen(time_now));
        }
};

//一定要inline：否则函数的参数file和line的默认__FILE__和__LINE__都是返回的这个hpp里的文件和行号
inline std::ostream& Log(LogLevel level,const std::string& log_msg,const char* file,int line)
{
    std::string level_info = Level[level];
    std::string time_now;
    LogTime::GetTime(&time_now);
    std::cout << "[" << time_now << " " << level_info << " " << file << ":" << line << "]" << log_msg;
    return std::cout;
}

#define LOG(level,msg) Log(level,msg,__FILE__,__LINE__)
