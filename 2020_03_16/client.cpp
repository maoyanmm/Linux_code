#include"udpsvr.hpp"

int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        printf("please : ./cli ip port\n");
        return 0;
    }
    std::string ip = argv[1];

    uint16_t port = atoi(argv[2]);

    UdpSvr us;
    if(!us.Create_Socket())
    {
        return 0;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    while(1)
    {
        std::string buf;
        printf("please iput message:\n");
        std::cin >> buf;
        us.Send_Message(buf,&addr);
        us.Receive_Message(buf,&addr);
        printf("server said:%s\n",buf.c_str());
    }
    us.Close();
    return 0;
}
