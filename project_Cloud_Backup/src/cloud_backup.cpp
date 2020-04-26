#include"cloud_backup.hpp"
#include<thread>

void test_hotcompress_start()
{
    _cloud_sys::NotHotCompress nc(COMMON_FILE_DIR,GZ_FILE_DIR);
    nc.Start();
    return;
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
