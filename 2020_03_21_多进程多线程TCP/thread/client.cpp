#pragma once
#include<stdio.h>
#include<iostream>
#include"tcp_socket.hpp"
#include<string>
#include<signal.h>

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "please input: ./cli ip port!" << std::endl;
        return 0;
    }
    std::string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    TcpSocket client_socket;
    if(!client_socket.Socket())
    {
        return 0;
    }
    if(!client_socket.Connect(ip,port))
    {
        return 0;
    }
    printf("i have connected server: ip[%s],port[%d]\n",ip.c_str(),port);
    while(1)
    {
        printf("please input: ");
        fflush(stdout);
        std::string buf;
        std::cin >> buf;
        client_socket.Send(buf);
        client_socket.Recv(buf);
        printf("server said: %s\n",buf.c_str());
    }
    client_socket.Close();
    return 0;
}
