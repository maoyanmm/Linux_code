#include<stdlib.h>
#include<vector>
#include<string>
#include<unistd.h>
#include<mysql/mysql.h>
#include<pthread.h>
#include<stdio.h>

#define MYSQL_IP "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PASSWD "1"
#define MYSQL_DB "file_name_db"

//前置声明
extern MYSQL* MysqlInit();
extern bool MysqlQuery(MYSQL* mysql, const std::string& sql);
extern void MysqlDestroy(MYSQL* mysql);
//1、mysql初始化
MYSQL* MysqlInit()
{
    //初始化mysql的句柄
    MYSQL* mysql = NULL;
    mysql = mysql_init(NULL);
    if(mysql == NULL)
    {
        printf("mysql init error\n");
        return NULL;
    }
    //连接服务器
    if(mysql_real_connect(mysql,MYSQL_IP,MYSQL_USER,MYSQL_PASSWD,MYSQL_DB,0,NULL,0) == NULL)
    {
        printf("connect mysql server failed : %s\n",mysql_error(mysql));
        MysqlDestroy(mysql);
        return NULL;
    }
    //设置客户端字符编码集
    if(mysql_set_character_set(mysql,"utf8") != 0)
    {
        printf("set mysql client character failed : %s\n",mysql_error(mysql));
        MysqlDestroy(mysql);
        return NULL;
    }
    return mysql;
}

//2、mysql执行语句
bool MysqlQuery(MYSQL* mysql, const std::string& sql)
{
    int ret = mysql_query(mysql, sql.c_str());
    if(ret != 0)
    {
        printf("sql:[%s] query failed : %s\n",sql.c_str(),mysql_error(mysql)); 
        return false;
    }
    return true;
}

//3、mysql销毁
void MysqlDestroy(MYSQL* mysql)
{
    if(mysql != NULL)
    {
        mysql_close(mysql);
    }
}

class FileNameManager
{
    private:
        static FileNameManager _fnm;//单例模式
        MYSQL* _mysql;//存储文件名-压缩文件名的数据库
        pthread_rwlock_t _rwlock;//保证修改数据库的文件名时候的安全
    public:
        //为了防构造、拷贝、赋值
        FileNameManager(MYSQL *mysql)
            :_mysql(mysql)
        {
            pthread_rwlock_init(&_rwlock,NULL);
        }
        FileNameManager(const FileNameManager& fnm);
        FileNameManager& operator=(const FileNameManager& fnm);
    public:
        static FileNameManager* GetFNM()
        {
            return &_fnm;
        }
        //判断文件是否存在
        bool IsExist(const std::string& file_name)
        {
            pthread_rwlock_rdlock(&_rwlock);
            //1、输入sql指令
            char buf[4069] = {0};
            sprintf(buf,"select file_name from file_gzfile_tb where file_name='%s';",file_name.c_str());
            if(MysqlQuery(_mysql,buf) == false)
            {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            //2、获取查询结果
            MYSQL_RES* res = mysql_store_result(_mysql);
            if(res == NULL)
            {
                printf("IsExist get one file_name result failed : %s\n",mysql_error(_mysql));
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            //3、判断结果是否存在
            int num_row = mysql_num_rows(res);
            if(num_row == 0)
            {
                //如果一条也没找到，则不存在
                pthread_rwlock_unlock(&_rwlock);
                mysql_free_result(res);
                return false;
            }
            pthread_rwlock_unlock(&_rwlock);
            mysql_free_result(res);
            return true;
        }
        //判断文件是否压缩
        bool IsCompress(const std::string& file_name)
        {
            //1、输入sql指令:找file_name文件的is_compress信息
            pthread_rwlock_rdlock(&_rwlock);
            char buf[4069] = {0};
            sprintf(buf,"select is_compress from file_gzfile_tb where file_name='%s';",file_name.c_str());
            if(MysqlQuery(_mysql,buf) == false)
            {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            //2、查询结果
            MYSQL_RES* res = mysql_store_result(_mysql);
            if(res == NULL)
            {
                printf("IsCompress get one file_name result failed : %s\n",mysql_error(_mysql));
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            int num_row = mysql_num_rows(res);
            if(num_row != 1)
            {
                printf("IsCompress one file_name result error\n");
                pthread_rwlock_unlock(&_rwlock);
                mysql_free_result(res);
                return false;
            }
            //3、判断文件是否压缩
            MYSQL_ROW row = mysql_fetch_row(res);
            if(std::stoi(row[0]) == 1)
            {
                pthread_rwlock_unlock(&_rwlock);
                mysql_free_result(res);
                return true;
            }
            pthread_rwlock_unlock(&_rwlock);
            mysql_free_result(res);
            return false;
        }
        //获得未压缩的文件列表
        bool GetUnCompressList(std::vector<std::string>* list)
        {
            //1、输入sql指令：找is_compress=0（没有压缩）的所有信息
            pthread_rwlock_rdlock(&_rwlock);
            char buf[4096] = {0};
            sprintf(buf,"select file_name from file_gzfile_tb where is_compress=%d",0);
            if(MysqlQuery(_mysql,buf) == false)
            {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            //2、查询结果
            MYSQL_RES* res = mysql_store_result(_mysql);
            if(res == NULL)
            {
                printf("GetUnCompressList get one file_name result failed : %s\n",mysql_error(_mysql));
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            //3、将查询结果插入list
            int num_rows = mysql_num_rows(res);
            for(int i = 0; i < num_rows; ++i)
            {
                //获取每一条信息（每个信息里只有一个file_name）
                MYSQL_ROW row = mysql_fetch_row(res);
                list->push_back(row[0]);
            }
            pthread_rwlock_unlock(&_rwlock);
            mysql_free_result(res);
            return true;
        }
        //获取所有文件名
        bool GetAllFile(std::vector<std::string>* list)
        {
            //1、输入sql指令：获取所有文件的文件名
            pthread_rwlock_rdlock(&_rwlock);
            if(MysqlQuery(_mysql,"select file_name from file_gzfile_tb;") == false)
            {
                return false;
            }
            //2、查询结果
            MYSQL_RES* res = mysql_store_result(_mysql);
            if(res == NULL)
            {
                printf("GetAllFile get one file_name result failed : %s\n",mysql_error(_mysql));
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            //3、将查询结果插入list
            int num_rows = mysql_num_rows(res);
            for(int i = 0; i < num_rows; ++i)
            {
                //获取每一条信息（一条信息只有一个file_name）
                MYSQL_ROW row = mysql_fetch_row(res);
                list->push_back(row[0]);
            }
            pthread_rwlock_unlock(&_rwlock);
            mysql_free_result(res);
            return true;
        }
        //插入一个文件信息
        bool Insert(const std::string& file_name)
        {
            pthread_rwlock_wrlock(&_rwlock);
            char buf[4096] = {0};
            //插入两个file_name、未压缩、存放时间
            sprintf(buf,"insert into file_gzfile_tb(file_name,is_compress,last_time) values('%s',%d,now());",
                    file_name.c_str(),0);
            if(MysqlQuery(_mysql,buf) == false)
            {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            pthread_rwlock_unlock(&_rwlock);
            return true;
        }
        //更新一个文件的文件信息 
        bool Update(const std::string& file_name, int is_compress)
        {
            //1、输入sql指令：更新文件信息（主要是指：更新is_compress、last_time）
            pthread_rwlock_wrlock(&_rwlock);
            char buf[4096] = {0};
            sprintf(buf,"update file_gzfile_tb set is_compress=%d, last_time=now() where file_name='%s'",
                    is_compress,file_name.c_str());
            if(MysqlQuery(_mysql,buf) == false)
            {
                pthread_rwlock_unlock(&_rwlock);
                return false;
            }
            pthread_rwlock_unlock(&_rwlock);
            return true;
        }
};
