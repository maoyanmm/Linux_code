#pragma once
#include<fstream>
#include<zlib.h>
#include<boost/filesystem.hpp>
#include<string>

namespace FileTool
{
    class FileReadWriteTool
    {
        public:
            //把file_name读到body里
            static bool FileRead(const std::string& file_name, std::string* body)
            {
                //1、开文件
                std::ifstream ifs(file_name,std::ios::binary);
                if(ifs.is_open() == false)
                {
                    printf("Readfile[%s] open failed!\n",file_name.c_str());
                    return false;
                }
                //2、计算文件大小
                int64_t fsize = boost::filesystem::file_size(file_name);
                //3、将body的大小扩容成file_size大小
                body->resize(fsize);
                //4、读文件
                ifs.read(&((*body)[0]),fsize);
                //判断上一次操作是否成功
                if(ifs.good() == false)
                {
                    printf("Readfile[%s] read failed!\n",file_name.c_str());
                    ifs.close();
                    return false;
                }
                ifs.close();
                return true;
            }
            //将body里的内容写到file_name里
            static bool FileWrite(const std::string& file_name, const std::string& body)
            {
                //1、开文件
                std::ofstream ofs(file_name,std::ios::binary);
                if(ofs.is_open() == false)
                {
                    printf("Writefile[%s] open failed!\n",file_name.c_str());
                    return false;
                }
                //2、写文件
                ofs.write(&(body[0]),body.size());
                if(ofs.good() == false)
                {
                    printf("Writefile[%s] write failed!\n",file_name.c_str());
                    ofs.close();
                    return false;
                }
                ofs.close();
                return true;
            }
    };
    class FileCompressTool
    {
        public:
            //将src压缩成gz_dst
            static bool Compress(const std::string& src, const std::string& gz_dst)
            {
                //1、读取src文件的内容
                std::string buf;
                FileReadWriteTool::FileRead(src,&buf);
                //2、以gzopen的写方式打开gz_dst
                gzFile gf = gzopen(gz_dst.c_str(),"wb");
                if(gf == NULL)
                {
                    printf("Compressfile[%s] gzopen failed!\n",src.c_str());
                    return false;
                }
                //3、压缩
                size_t wlen = 0;//表示已经压缩了的字节数
                while(wlen < buf.size())
                {
                    //可能只是压缩部分，所以要循环压缩，每次压缩的大小有变化
                    size_t tmp_len = gzwrite(gf,&buf[wlen],buf.size()-wlen);
                    if(tmp_len == 0)
                    {
                        printf("Compressfile[%s] compress failed!\n",src.c_str());
                    }
                    //每次wlen都加上已经压缩了的长度
                    wlen += tmp_len;
                }
                gzclose(gf);
                return true;
            }
            static bool UnCompress(const std::string& gz_src,const std::string& dst)
            {
                //1、以写打开dst文件
                std::ofstream ofs(dst,std::ios::binary);
                if(ofs.is_open() == false)
                {
                    printf("UnCompressfile[%s] open failed!\n",dst.c_str());
                    return false;
                }
                //2、以gzopen的读方式打开gz_src
                gzFile gf = gzopen(gz_src.c_str(),"rb");
                if(gf == NULL)
                {
                    printf("UnCompressfile[%s] gzopen failed!\n",gz_src.c_str());
                    ofs.close();
                    return false;
                }
                //3、解压缩
                char buf[4096] = {0};//单次解压的容器
                size_t ret = 0;//表示单次解压了多少
                while((ret = gzread(gf,buf,sizeof(buf))) > 0)
                {
                    //处于内存的考虑，每次解压缩buf大小，然后读进去
                    //这里不用自己写的FileWrite的原因是：
                    //因为每次只写一部分，而每次进入这个函数写的时候都是从开头开始写；
                    //但是如果把自己写的函数改成追加方式的话,会导致后期传同名文件的时候不是覆盖而是追加
                    ofs.write(buf,ret);
                }
                ofs.close();
                gzclose(gf);
                return true;
            }
    };
}
