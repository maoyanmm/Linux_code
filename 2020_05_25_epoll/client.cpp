#include"tcp_socket.hpp"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("please input: ./cli ip port\n");
        return 0;
    }
    std::string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    TcpSocket cli;
    if(!cli.Socket())
    {
        return 0;
    }
    if(!cli.Connect(ip,port))
    {
        return 0;
    }
    while(1)
    {
        printf("please input message:");
        fflush(stdout);
        std::string str;
        std::cin >> str;
        cli.Send(str);
    }
   return 0; 
}
