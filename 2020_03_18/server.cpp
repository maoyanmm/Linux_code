#include"tcp_socket.hpp"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("please input: ./srv ip port\n");
        return 0;
    }
    std::string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    TcpSocket srv;
    if(!srv.Socket())
    {
        return 0;
    }
    if(!srv.Bind(ip,port))
    {
        return 0;
    }
    if(!srv.Listen())
    {
        return 0;
    }
    while(1)
    {
        TcpSocket accept;
        srv.Accept(accept,&ip,&port);
        printf("i have accept a client : ip[%s] port[%d]",ip.c_str(),port);
        std::string buf;
        accept.Recv(buf);
        printf("client said:%s\n",buf.c_str());
        printf("please input message:");
        fflush(stdout);
        std::cin >> buf;
        accept.Send(buf);
    }
    srv.Close();
    return 0;
}
