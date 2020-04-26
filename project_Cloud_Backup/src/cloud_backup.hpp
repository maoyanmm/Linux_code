#include<cstdio>
#include<sys/stat.h>
#include<unistd.h>
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<unordered_map>
#include<pthread.h>
#include<zlib.h>
#include<boost/filesystem.hpp>
#include<boost/algorithm/string.hpp>
#include"httplib.h"

#define NOT_HOT_TIME 15 //最后一次访问距现在的时间
#define CHECK_INTVAL_TIME 30 //检查的间隔的时间
#define COMMON_FILE_DIR "./common/" //未压缩的文件存放的目录
#define GZ_FILE_DIR "./gz/" //压缩的文件存放的目录
#define FILE_NAME_DIR "./fileName" //所有文件名存放的目录

namespace _cloud_sys
{

    //文件的工具类（读，写）
    class FileUtil
    {
        public:
            //从文件中读取内容
            static bool Read(const std::string& name, std::string* body)
            {
                std::ifstream ifs(name,std::ios::binary);
                if(ifs.is_open() == false)
                {
                    printf("file %s open failed!\n",name.c_str());
                    return false;
                }
                //得到该文件的大小
                int64_t fsize = boost::filesystem::file_size(name);
                //将要接受的容器扩容
                body->resize(fsize);
                ifs.read(&((*body)[0]),fsize);
                //判断上次操作是否成功
                if(ifs.good() == false)
                {
                    printf("file %s read failed!\n",name.c_str());
                    return false;
                }
                ifs.close();
                return true;
            }
            static bool Write(const std::string& name, const std::string& body)
            {
                std::ofstream ofs(name,std::ios::binary);
                if(ofs.is_open() == false)
                {
                    printf("Write : file %s open failed!\n",name.c_str());
                    return false;
                }
                ofs.write(&body[0],body.size());
                if(ofs.good() == false)
                {
                    printf("file %s write failed!\n",name.c_str());
                    return false;
                }
                ofs.close();
                return true;
            }
    };
    class CompressUtil
    {
        public:
            //将src文件压缩到dst
            static bool Compress(const std::string& src, const std::string& dst)
            {
                //先读源文件
                std::string buf;
                FileUtil::Read(src,&buf);
                //以写入（压缩）的方式打开最后被压缩的目标文件 
                gzFile gf = gzopen(dst.c_str(),"wb");
                if(gf == NULL)
                {
                    printf("file %s open failed!\n",dst.c_str());
                    return false;
                }
                size_t wlen = 0;//表示压缩了文件的多少字节长度
                //直到整个buf被压缩完
                while(wlen < buf.size())
                {
                    //可能一次没有压缩完，该函数返回这一次压缩的长度
                    int clen = gzwrite(gf,&buf[wlen],buf.size()-wlen);
                    if(clen == 0)
                    {
                        printf("file %s write compress failed!\n",dst.c_str());
                    }
                    wlen += clen;
                }
                gzclose(gf);
                return true;
            }
            //将src解压到dst
            static bool UnCompress(const std::string& src, const std::string& dst)
            {
                //打开装 解压后的文件数据的 文件
                std::ofstream ofs(dst,std::ios::binary);
                if(ofs.is_open() == false)
                {
                    printf("file %s open failed!\n",dst.c_str());
                    return false;
                }
                //打开要压缩的文件
                gzFile gf = gzopen(src.c_str(),"rb");
                if(gf == NULL)
                {
                    printf("file %s gzopen failed!\n",src.c_str());
                    //如果这个gz文件打开失败，但是之前的ofstream文件已经打开，要关掉
                    ofs.close();
                    return false;
                }
                size_t ret = 0;
                char buf[4096] = {0};
                //每次读取一定量的数据
                while((ret = gzread(gf,buf,sizeof(buf))) > 0)
                {
                    //将每一次读取到的解压后的数据放入目标文件
                    ofs.write(buf,ret);
                }
                ofs.close();
                gzclose(gf);
                return true;
            }
    };
    class DataManager
    {
        private:
            DataManager(const std::string& path)
                :_back_filename(path)
            {
                pthread_rwlock_init(&_rwlock,NULL);
                InitLoad();
            }
            DataManager(const DataManager& dm);
            DataManager& operator=(const DataManager& dm);
        public:
            static DataManager* GetDM()
            {
                return &_dm;
            }
            ~DataManager()
            {
                pthread_rwlock_destroy(&_rwlock);
            }
            //检查文件是否存在
            bool Exist(const std::string& name)
            {
                pthread_rwlock_rdlock(&_rwlock);
                auto it = _file_list.find(name);
                if(it == _file_list.end())
                {
                    pthread_rwlock_unlock(&_rwlock);
                    return false;
                }
                pthread_rwlock_unlock(&_rwlock);
                return true;
            }
            //检查文件是否已压缩
            bool IsCompress(const std::string& name)
            {
                pthread_rwlock_rdlock(&_rwlock);
                auto it = _file_list.find(name);
                if(it == _file_list.end())
                {
                    pthread_rwlock_unlock(&_rwlock);
                    return false;
                }
                //如果文件未压缩，那么这个键值对的两个字符串都是那个文件的文件名
                //如果压缩了，那么键值对的第一个是原文件名，第二个是压缩后的文件名
                if(it->first != it->second)
                {
                    pthread_rwlock_unlock(&_rwlock);
                    return true;
                }
                else
                {
                    pthread_rwlock_unlock(&_rwlock);
                    return false;
                }
            }
            //获取未压缩的文件列表
            bool GetUnCompressList(std::vector<std::string>* list)
            {
                pthread_rwlock_rdlock(&_rwlock);
                for(auto it = _file_list.begin(); it != _file_list.end(); ++it)
                {
                    if(it->first == it->second)
                    {
                        list->push_back(it->first);
                    }
                }
                pthread_rwlock_unlock(&_rwlock);
                return true;
            }
            //更新文件名（压缩后将src的second改成dst，src和dst不一样)
            //插入文件名（刚上传文件，将文件名插入到list，src和dst一样）
            bool Insert(const std::string& src,const std::string& dst)
            {
                pthread_rwlock_wrlock(&_rwlock);
                _file_list[src] = dst;
                pthread_rwlock_unlock(&_rwlock);
                printf("Insert\n");
                for(auto it = _file_list.begin(); it != _file_list.end(); ++it)
                {
                    std::cout << it->first << "-->" << it->second << std::endl;
                }
                Storage();//存到磁盘里
                return true;
            }
            //获取所有文件（向客户展示文件列表，所以不展示压缩后的文件）
            bool GetAllFile(std::vector<std::string>* list)
            {
                pthread_rwlock_wrlock(&_rwlock);
                for(auto it = _file_list.begin(); it != _file_list.end(); ++it)
                {
                    list->push_back(it->first);
                }
                pthread_rwlock_unlock(&_rwlock);
                return true;
            }
            //获取压缩包名称
            bool GetGzName(const std::string& src, std::string* dst)
            {
               auto it = _file_list.find(src);
               if(it == _file_list.end())
               {
                   printf("GetGzName failed!\n");
                   return false;
               }
               *dst = it->second;
               return true;
            }
            //将接收到的文件名存在磁盘里
            bool Storage()
            {
                std::stringstream tmp;
                pthread_rwlock_rdlock(&_rwlock);
                for(auto it = _file_list.begin(); it != _file_list.end(); ++it)
                {
                    //将所有文件名（包括对应的压缩包名）输入到string流
                    tmp << it->first << " " << it->second << "\r\n";
                }
                //将所有文件名（包括对应的压缩包名）写到磁盘的文件里
                FileUtil::Write(_back_filename,tmp.str());                
                pthread_rwlock_unlock(&_rwlock);
                
                printf("Storage\n");
                for(auto it = _file_list.begin(); it != _file_list.end(); ++it)
                {
                    std::cout << it->first << "-->" << it->second << std::endl;
                }
                return true;
            }
            //将磁盘的文件加载
            bool InitLoad()
            {
                //先将所有的文件名读出来
                std::string buf;
                if(FileUtil::Read(_back_filename,&buf) == false)
                {
                    return false;
                }
                //再将所有的文件名（包括对应的压缩包文件名）一个个提取出来(各个文件之间用\r\n分割)
                std::vector<std::string> list;
                boost::algorithm::split(list,buf,boost::is_any_of("\r\n"),
                        boost::token_compress_off);
                //原文件和压缩的文件名用空格分割，需要另外分开它们，存储到_file_list
                for(auto& e:list)
                {
                    size_t  pos = e.find(" ");
                    if(pos == std::string::npos)
                    {
                        continue;
                    }
                    std::string file_name = e.substr(0,pos);
                    std::string file_etag = e.substr(pos+1);
                    //分割完key和value后插入到_file_list
                    _file_list[file_name] = file_etag;
                }
                return true;
            }
        private:
            //storage里用到的，将文件名存在磁盘里的_back_file文件
            std::string _back_filename;
            //数据管理容器
            std::unordered_map<std::string,std::string> _file_list;
            //读写锁
            pthread_rwlock_t _rwlock;
            static DataManager _dm;
    };
    DataManager DataManager::_dm(FILE_NAME_DIR);
    
    class NotHotCompress
    {
        public:
            NotHotCompress(const std::string& co_dir,const std::string& gz_dir)
                :_gz_dir(gz_dir)
                 ,_co_dir(co_dir)
            {  }
            bool Start()
            {
                while(1)
                {
                    //获取未压缩的文件列表
                    std::vector<std::string> list;
                    DataManager::GetDM()->GetUnCompressList(&list);
                    for(const auto& file_name:list)
                    {
                        std::string path_file_name = _co_dir + file_name;
                        //循环判断这些未压缩的文件是否是非热点文件
                        if(IsHot(path_file_name) == false)
                        {
                            //如果是非热点文件则压缩
                            std::string dst_name = _gz_dir + file_name + ".gz";
                            if(CompressUtil::Compress(path_file_name,dst_name) == true)
                            {
                                //压缩完就更新文件名List的键值对
                                DataManager::GetDM()->Insert(file_name,file_name+".gz");
                                //压缩完，原文件就没用了，删除掉，但是_file_list里的文件名并不删除
                                unlink(path_file_name.c_str());
                            }
                        }       
                    }
                    //每过一定的时间就检查一次
                    sleep(CHECK_INTVAL_TIME);
                }
            }
        private:
            bool IsHot(const std::string& name)
            {
                //获取当前时间
                time_t cur_time = time(NULL);
                //获取最后一次修改时间的结构体
                struct stat st;
                if(stat(name.c_str(),&st) < 0)
                {
                    printf("get file %s stat failed!\n",name.c_str());
                    return false;
                }
                if((cur_time - st.st_atime) > NOT_HOT_TIME)
                {
                    return false;
                }
                return true;
            }
        private:
            std::string _gz_dir;//压缩后的文件存储路径
            std::string _co_dir;//压缩前的文件存储路径 
    };
    class Server
    {
        public:
            bool Start()
            {
                _server.Put("/(.*)",Upload);
                _server.Get("/list",List);
                //.*表示匹配任意字符串，()表示捕捉这个字符串
                _server.Get("/download/(.*)",Download);
                _server.listen("0.0.0.0",4418);
                return true;
            }
        private:
            static void Upload(const httplib::Request& req, httplib::Response& rsp)
            {
                std::string file_name = req.matches[1];
                std::string path_file_name = COMMON_FILE_DIR + file_name;
                FileUtil::Write(path_file_name,req.body);
               
                rsp.status = 200;
                return ;
            }
            static void List(const httplib::Request& req, httplib::Response& rsp)
            {
                //获取所有文件的列表
                std::vector<std::string> list;
                DataManager::GetDM()->GetAllFile(&list);
                std::stringstream tmp;
                tmp << "<html><body><hr />";
                for(size_t i = 0; i < list.size(); ++i)
                {
                    //href后跟的是超链接,第一个list是超链接里路径下的文件,第二个是展示的字符串
                    //点击list后，会给服务器发送/Download/list[i]请求
                    tmp << "<a href='/download/" << list[i] << "'>" << list[i] << "</a>";
                    tmp << "<hr />";
                }
                tmp << "<hr /></body></html>"; 
                //将这个tmp字符串传入正文
                rsp.set_content(tmp.str().c_str(),tmp.str().size(),"text/html");
                rsp.status = 200;
                return;
            }
            static void Download(const httplib::Request& req, httplib::Response& rsp)
            {
                std::string file_name = req.matches[1];//matches是捕捉(.*)
                //先查看文件是否存在
                if(DataManager::GetDM()->Exist(file_name) == false)
                {
                    rsp.status = 404;
                    return;
                }
                std::string path_file_name = COMMON_FILE_DIR + file_name; 
                //再检查文件是否在磁盘已经被压缩
                //如果被压缩，则解压缩
                if(DataManager::GetDM()->IsCompress(file_name) == true)
                {
                    std::string gzfile;
                    DataManager::GetDM()->GetGzName(file_name,&gzfile);
                    std::string path_gzfile_name = GZ_FILE_DIR + gzfile; 
                    CompressUtil::UnCompress(path_gzfile_name,path_file_name);
                    //解压后删除压缩包
                    unlink(path_gzfile_name.c_str());
                    //解压缩后更新文件列表
                    DataManager::GetDM()->Insert(file_name,file_name);
                }
                //把原文件的数据传入rsp的正文
                FileUtil::Read(path_file_name,&rsp.body);
                rsp.set_header("content-type","application/octet-stream");//二进制流下载
                rsp.status = 200;
                return;
            }
        private:
            std::string _file_dir;
            httplib::Server _server;
    };
}
