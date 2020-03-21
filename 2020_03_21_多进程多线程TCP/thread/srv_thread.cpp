#include<stdio.h>
#include<iostream>
#include"tcp_socket.hpp"
#include<string>
#include<signal.h>
#include<pthread.h>

void* ThreadStart(void* arg)
{
    TcpSocket* accept_socket = (TcpSocket*)arg;
    while(1)
    {
        std::string buf;
        accept_socket->Recv(buf);
        printf("client said: [%s]\n",buf.c_str());
        printf("server input:");
        fflush(stdout);
        std::cin >> buf;
        accept_socket->Send(buf);
    }
    accept_socket->Close();
    delete accept_socket;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "please input: ./srv ip port!" << std::endl;
        return 0;
    }
    std::string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    TcpSocket listen_socket;
    if(!listen_socket.Socket())
    {
        return 0;
    }
    if(!listen_socket.Bind(ip,port))
    {
        return 0;
    }
    while(1)
    {
        listen_socket.Listen(5);
        TcpSocket* accept_socket = new TcpSocket;
        std::string cli_ip;
        uint16_t cli_port;
        if(!listen_socket.Accept(accept_socket,&cli_ip,&cli_port))
        {
            //如果完成的连接队列里面没有已经完成的连接，那就跳过这次
            continue;
        }
        printf("sever have connected a client: ip[%s],port[%d]\n",cli_ip.c_str(),cli_port);

        pthread_t tid;
        int ret = pthread_create(&tid,NULL,ThreadStart,(void*)accept_socket); 
        if(ret < 0)
        {
            perror("pthread_create");
            return 0;
        }
    }
    listen_socket.Close();
    return 0;
}
