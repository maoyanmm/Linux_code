#pragma once
#include<sys/stat.h>
#include<pthread.h>
#include"FileNameManager.hpp"
#include"FileTool.hpp"
#include"httplib.h"

#define COMMON_FILE_DIR "./common_file/"//原文件存放的文件夹
#define GZ_FILE_DIR "./gz_file/"//压缩文件存放的文件夹
#define SERVER_IP "0.0.0.0"
#define SERVER_PORT 4418

class Server
{
    private:
        httplib::Server _server;
    public:
        void Start()
        {
            //上传文件
             _server.Put("/(.*)",Upload);
            //请求文件列表
            _server.Get("/list",List);
            //下载文件
            _server.Get("/download/(.*)",Download);
            //监听连接
            _server.listen(SERVER_IP,SERVER_PORT);
        }
    private:
        static void Upload(const httplib::Request& request, httplib::Response& response) 
        {
            //1、拿到带路径的文件名
            std::string file_name = request.matches[1];
            std::string path_file_name = COMMON_FILE_DIR + file_name;
            printf("file[%s] is upload ... ...\n",file_name.c_str());
            //2、更新数据库里的文件信息
            //如果文件不存在则插入文件信息
            if(FileNameManager::GetFNM()->IsExist(file_name) == false)
            {
                FileNameManager::GetFNM()->Insert(file_name);
            }
            //如果文件存在，将之前的文件删除，然后传入新的文件，更新信息
            else
            {
                //如果之前的文件被压缩了，则删除压缩包
                if(FileNameManager::GetFNM()->IsCompress(file_name) == true)
                {
                    std::string path_gzfile_name = GZ_FILE_DIR + file_name + ".gz";
                    unlink(path_gzfile_name.c_str());
                }
                //如果之前的文件还没被压缩，则删除原文件
                else
                {
                    unlink(path_file_name.c_str());
                }
                FileNameManager::GetFNM()->Update(file_name,0);
            }
            //3、将文件数据写入到磁盘里
            FileTool::FileReadWriteTool::FileWrite(path_file_name,request.body);
            //4、设置状态码
            response.status = 200;
            printf("file[%s] upload success!\n",file_name.c_str());
        }
        static void List(const httplib::Request& request, httplib::Response& response)
        {
            (void)request;
            //1、获取所有文件名的列表
            std::vector<std::string> list;
            FileNameManager::GetFNM()->GetAllFile(&list); 
            //2、遍历这个列表，把所有文件名和对应的下载链接填入到正文中
            std::stringstream tmp; 
            tmp << "<html><body><hr />";
            for(const auto& file_name:list)
            {
                //第一个file_name是请求下载file_name的请求方法，可点击来发送请求。第二个是展示页面的文件名
                tmp << "<a href='/download/" << file_name << "'>" << file_name << "</a>";
                tmp << "<hr />";
            }
            tmp << "<hr /></body><html>";
            //3、设置应答正文信息和状态码
            response.set_content(tmp.str().c_str(),tmp.str().size(),"text/html");
            response.status = 200;
        }
        static void Download(const httplib::Request& request, httplib::Response& response)
        {
            //1、获取要下载的文件名
            std::string file_name = request.matches[1];
            std::string path_file_name = COMMON_FILE_DIR + file_name;
            //2、检查文件是否存在
            if(FileNameManager::GetFNM()->IsExist(file_name) == false)
            {
                response.status = 404;
                return;
            }
            //3、检查文件是否已压缩
            if(FileNameManager::GetFNM()->IsCompress(file_name) == true)
            {
                //如果压缩了就解压缩
                std::string path_gzfile_name = GZ_FILE_DIR + file_name + ".gz";
                FileTool::FileCompressTool::UnCompress(path_gzfile_name,path_file_name);
                //解压完删除压缩包
                unlink(path_gzfile_name.c_str());
                //删除了解压包后更新文件的信息(0表示未压缩)
                FileNameManager::GetFNM()->Update(file_name,0);
            }
            //4、将客户端要下载的文件数据写到response的正文里
            FileTool::FileReadWriteTool::FileRead(path_file_name,&response.body);
            response.set_header("content-type","application/octet-stream");//二进制流下载
            response.status = 200;
        }
};
