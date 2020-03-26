//日志文件
#include<iostream>
#include<string.h>
#include<string>

//用来表示错误数组的下标
enum LogLevel
{
    INFO = 0,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};
const char* Level[] = {"INFO","WARNING","ERROR","FATAL","DEBUG"};

class LogTime
{
public:
    static void GetTime(std::string& time_stamp)
    {
        //获取时间戳
        time_t sys_time;
        time(&sys_time);
        //将时间划分为：年月日 时分秒
        struct tm* st = localtime(&sys_time);
        char time_now[25] = {'\0'};
        //将时间按照格式放到数组里
        snprintf(time_now,sizeof(time_now)-1,"%04d-%02d-%02d %02d:%02d:%02d",st->tm_year+1900,st->tm_mon+1,st->tm_mday,st->tm_hour,st->tm_min,st->tm_sec);
        time_stamp.assign(time_now,strlen(time_now));
    }
};

//为了打印出：[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息
inline void Log(LogLevel lev,const char* file,int line, std::string log_msg)
{
    std::string time_stamp;
    std::string level_info = Level[lev];
    LogTime::GetTime(time_stamp);

    std::cout << "[" << time_stamp << " " << level_info << " " << file << " " << line << "]" << log_msg << std::endl; 
}

#define LOG(lev,msg) Log(lev,__FILE__,__LINE__,msg) 
