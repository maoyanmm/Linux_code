#include<thread>
#include"HttpServer.hpp"
#include"NotHotCompress.hpp"

void Server_Start()
{
    Server server;
    server.Start();
}
void Compress_start()
{
    NotHotCompress nhc;
    nhc.Start();
}

int main()
{
    std::thread thread_server(Server_Start);
    std::thread thread_not_hot_compress(Compress_start);
    thread_server.join();
    thread_not_hot_compress.join();
    return 0;
}
