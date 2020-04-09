#pragma once
#include"FileNameManager.hpp"
#include"FileTool.hpp"
#include<time.h>
#include<pthread.h>

#define COMMON_FILE_DIR "./common_file/"//原文件存放的文件夹
#define GZ_FILE_DIR "./gz_file/"//压缩文件存放的文件夹
#define CHECK_TIME 30//检查的间隔时间
#define NOT_HOT_TIME 15//非热点文件的时间标准

class NotHotCompress
{
    private:
        std::string _co_file_dir;
        std::string _gz_file_dir;
    public:
        NotHotCompress(const std::string& co_file_dir = COMMON_FILE_DIR, const std::string& gz_file_dir = GZ_FILE_DIR)
            :_co_file_dir(co_file_dir)
             ,_gz_file_dir(gz_file_dir)
        {  }
        bool Start()
        {
            while(1)
            {
                //1、拿到所有未压缩的文件名
                std::vector<std::string> list;
                //2、循环的判断每一个文件是否是热点文件，然后进行压缩
                FileNameManager::GetFNM()->GetUnCompressList(&list);
                for(const auto& file_name:list)
                {
                    //如果是热点文件则不需要压缩
                    if(IsHot(file_name) == true)
                    {
                        continue;
                    }
                    //拿到带路径的原文件的文件名、带路径的压缩文件的文件名
                    std::string gzfile_name = file_name + ".gz";
                    std::string path_gzfile_name = _gz_file_dir + gzfile_name;
                    std::string path_file_name = _co_file_dir + file_name;
                    //如果压缩失败则跳过。但并不表示不压缩了，因为下次while循环还会回来的
                    if(FileTool::FileCompressTool::Compress(path_file_name,path_gzfile_name) == false)
                    {
                        continue;
                    }
                    //更新数据库的文件信息
                    FileNameManager::GetFNM()->Update(file_name,1);
                    //删除原文件，只保留压缩文件
                    unlink(path_file_name.c_str());
                }
                //每隔一定的时间检查一次
                sleep(CHECK_TIME);
            }
        }
        bool IsHot(const std::string& file_name)
        {
            long last_time;
            FileNameManager::GetFNM()->GetFileTime(file_name,&last_time); 
            time_t now_time = time(NULL);
            if(now_time - last_time > NOT_HOT_TIME)
            {
                return false;
            }
            return true;
        }
};
