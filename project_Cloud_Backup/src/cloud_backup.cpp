#include"cloud_backup.hpp"
#include<thread>

void test_compress(char* argv[])
{
    _cloud_sys::CompressUtil::Compress(argv[1],argv[2]);
    std::string tmp = argv[2];
    tmp += ".txt";
    _cloud_sys::CompressUtil::UnCompress(argv[2],tmp);
}

void test_storage()
{
    _cloud_sys::DataManager dm("./test.txt");
    //dm.Insert("a.txt","a.txt");
    //dm.Insert("b.txt","b.txt.gz");
    //dm.Insert("c.txt","c.txt.gz");
    //dm.Insert("d.txt","d.txt");
    //dm.Storage();
    std::vector<std::string> list;
    dm.InitLoad();
//    dm.Insert("a.txt","a.txt.gz");
    dm.GetAllFile(&list);
    for(const auto& e:list)
    {
        std::cout << e << std::endl;
    }
    std::cout << "-----------------" << std::endl;
    list.clear();
    dm.GetUnCompressList(&list);
    for(const auto& e:list)
    {
        std::cout << e << std::endl;
    }

}
void test_hotcompress_start()
{
    _cloud_sys::NotHotCompress nc(COMMON_FILE_DIR,GZ_FILE_DIR);
    nc.Start();
    return;
}

void test_hotcompress()
{
    if(boost::filesystem::exists(GZ_FILE_DIR) == false)
    {
        boost::filesystem::create_directory(GZ_FILE_DIR);
    }
    if(boost::filesystem::exists(COMMON_FILE_DIR) == false)
    {
        boost::filesystem::create_directory(COMMON_FILE_DIR);
    }
    _cloud_sys::dm.Insert("abcd.txt","abcd.txt");
    std::thread thr(test_hotcompress_start);
    thr.join();
}

void test_server_start()
{
    _cloud_sys::Server server;
    server.Start();
    return;
}
void test_server()
{
    std::thread thr_compress(test_hotcompress_start);
    std::thread thr_server(test_server_start);
    thr_compress.join();
    thr_server.join();
}

int main(int argc, char* argv[])
{
    test_server();
    return 0;
}
