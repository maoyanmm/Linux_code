#include"udpsvr.hpp"

int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        printf("please iput: ./srv ip port\n");
        return 0;
    }
    std::string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    UdpSvr us;
    if(!us.Create_Socket())
    {
        return 0;
    }
    if(!us.Bind(ip,port))
    {
        printf("the port number has been used\n");
        return 0;
    }

    while(1)
    {
        std::string buf;
        struct sockaddr_in addr;
        us.Receive_Message(buf,&addr);
        printf("cliend said:%s\n",buf.c_str());
        printf("please input message:\n");
        std::cin >> buf;
        us.Send_Message(buf,&addr);

    }
    us.Close();
    return 0;
}
