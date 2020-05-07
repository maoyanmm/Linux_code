#pragma once
#include<mysql/mysql.h>
#include<string>
#include<cstring>
#include<vector>
#include<unordered_map>
#include<set>
#include<utility>
#include<unistd.h>
#include<arpa/inet.h>

#include"Log.hpp"
#include"ConnectInfo.hpp"

#define MYSQL_IP "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PASSWORD "1"
#define MYSQL_DB "user_info_db"
#define MYSQL_TB "user_info_tb"

//该用户信息管理模块是用来将注册了的、登陆了的用户信息进行管理（登陆、注册）

//表示当前用户的状态
enum UserStatus
{
    OFFLINE = 0,//下线
    ONLINE,//上线
    WILL_BE_ONLINE//即将上线，这个状态是TCP验证密码通过后的状态，客户端即将发送udp数据，然后获取到udp地址才算正式登陆
};

class Mysql
{
    public:
        static MYSQL* MysqlInit()
        {
            //1、初始化mysql句柄
            MYSQL* mysql = NULL;
            mysql = mysql_init(NULL);
            if(mysql == NULL)
            {
                LOG(FATAL,"mysql_init failed!") << std::endl;
                return NULL;
            }

            //2、连接服务器
            if(mysql_real_connect(mysql,MYSQL_IP,MYSQL_USER,MYSQL_PASSWORD,MYSQL_DB,0,NULL,0) == NULL)
            {
                LOG(FATAL,"mysql_real_connect failed!") << std::endl;
                MysqlDestory(mysql); 
                return NULL;
            }
            //3、设置客户端字符编码集
            if(mysql_set_character_set(mysql,"utf8") != 0)
            {
                LOG(FATAL,"mysql_set_character_set failed!") << std::endl;
                MysqlDestory(mysql); 
                return NULL;
            }
            return mysql;
        }

        static bool MysqlQuery(MYSQL* mysql,const std::string& sql_query)
        {
            int ret = mysql_query(mysql,sql_query.c_str());
            if(ret != 0)
            {
                LOG(FATAL,"MysqlQuery failed!") << std::endl;
                return false;
            }
            return true;
        }

        static void MysqlDestory(MYSQL* mysql)
        {
            if(mysql != NULL)
            {
                mysql_close(mysql);
            }
        }
};

class UserInfo
{
    public:
        //基本信息
        std::string _nick_name;  
        std::string _birthday;
        std::string _password;
        uint32_t _user_id;
        //聊天时候用的udp信息
        struct sockaddr_in _addr;
        socklen_t _addr_len;
        UserStatus _status;
    public:
        UserInfo(const std::string& nick_name,const std::string& birthday,const std::string& password,uint32_t user_id)
            :_nick_name(nick_name),_birthday(birthday),_password(password),_user_id(user_id)
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
        std::vector<UserInfo> _online_list;
        //预分配的用户id
        uint32_t _prepare_user_id;
        //管理用户信息的数据库
        MYSQL* _mysql;
    public:
        UserManager()
        {
            _mysql = Mysql::MysqlInit();
            if(InitUserListInfo() == false)   
            {
                exit(20);
            }
            pthread_mutex_init(&_lock,NULL);
        }
        ~UserManager()
        {
            pthread_mutex_destroy(&_lock);
            Mysql::MysqlDestory(_mysql);
        }
        bool Register(const std::string nick_name,const std::string& birthday, const std::string& password,uint32_t* id)
        {
            //如果输入的注册信息为空，则注册失败
            if(nick_name.size() == 0 || birthday.size() == 0 || password.size() == 0)
            {
                LOG(ERROR,"Register failed!") << std::endl;
                return false;
            }
            pthread_mutex_lock(&_lock);
            //1、创建一个用户信息对象，并将注册的信息填入
            UserInfo user_info(nick_name,birthday,password,_prepare_user_id);
            user_info._status = OFFLINE;
            //2、将这个用户信息插入到用户信息列表里
            _user_info_list.insert(std::make_pair(_prepare_user_id,user_info));
            //2.1、将用户信息插入到数据库里
            InsertUserInfo(user_info); 
            //3、返回注册得到的新ID
            *id = _prepare_user_id;
            //4、将预分配的id++
            ++_prepare_user_id;
            LOG(INFO,"Register a user success") << std::endl;
            pthread_mutex_unlock(&_lock);
            return true;
        }
        bool Login(uint32_t user_id,const std::string& password,ReplyInfo* response)
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
            //登陆成功后，用户从服务端得到自己的信息（因为自己的信息肯定是存在服务端的，而不是客户端
            strcpy(response->_nick_name,it->second._nick_name.c_str());
            strcpy(response->_birthday,it->second._birthday.c_str());
            response->_user_id = it->second._user_id;
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
                memcpy(&it->second._addr,&cli_addr,sizeof(cli_addr));
                it->second._addr_len = addr_len;
                _online_list.push_back(it->second);
                LOG(INFO,"user Login success!") << std::endl;
                pthread_mutex_unlock(&_lock);
                //这里返回false是表示不需要把这条消息放入消息池里
                return false;
            }
            pthread_mutex_unlock(&_lock);
            return true; 
        }
        void GetOnlinelist(std::vector<UserInfo>* online_list)
        {
             *online_list = _online_list;
        }
    private:
        //数据库处理
        //得到给用户注册时候用到的预分配账号,并且把数据库的用户信息加载到内存
        bool InitUserListInfo()
        {
            //1、输入mysql查询语句:查找所有信息
            char buf[1024] = {0};
            sprintf(buf,"select * from %s;",MYSQL_TB);
            if(Mysql::MysqlQuery(_mysql,buf) == false)
            {
                return false;
            }
            //2、查询结果
            MYSQL_RES* res = mysql_store_result(_mysql);
            if(res == NULL)
            {
                LOG(FATAL,"GetPrepareNum get result failed!") << std::endl;
                return false;
            }
            //2.1、将用户数量赋值给预分配id
            int num_rows = mysql_num_rows(res);
            _prepare_user_id = num_rows; 
            //2.2、将所有用户信息加载到map
            for(int i = 0; i < num_rows; ++i)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                uint32_t id = std::stoi(row[3]);
                UserInfo ui(row[0],row[1],row[2],id);
                _user_info_list.insert(std::make_pair(id,ui));
            }
            mysql_free_result(res);
            return true;
        }
        bool InsertUserInfo(const UserInfo& user_info)
        {
            //输入mysql插入语句:插入一个用户信息
            char buf[1024] = {0};
            sprintf(buf,"insert into %s values('%s','%s','%s',%d);",MYSQL_TB,user_info._nick_name.c_str(),user_info._birthday.c_str(),user_info._password.c_str(),user_info._user_id);
            if(Mysql::MysqlQuery(_mysql,buf) == false)
            {
                return false;
            }
            return true;
        }
        
};
