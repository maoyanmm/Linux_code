#include<stdio.h>
#include<iostream>
#include"tcp_socket.hpp"
#include<string>
#include<signal.h>
#include<sys/wait.h>

void Sigcb(int sig)
{
    (void)sig;
    while(1)
    {
        waitpid(-1,NULL,WNOHANG);
    }
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "please input: ./srv ip port!" << std::endl;
        return 0;
    }
    signal(SIGCHLD,Sigcb);
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
        TcpSocket accept_socket;
        std::string cli_ip;
        uint16_t cli_port;
        if(!listen_socket.Accept(accept_socket,&cli_ip,&cli_port))
        {
            //如果完成的连接队列里面没有已经完成的连接，那就跳过这次
            continue;
        }
        printf("sever have connected a client: ip[%s],port[%d]\n",cli_ip.c_str(),cli_port);
        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork");
            exit(0);
        }
        else if(pid == 0)
        {
            //接待的套接字负责和一个客户端进行交互
            while(1)
            {
                std::string buf;
                accept_socket.Recv(buf);
                printf("client said: %s\n",buf.c_str());
                printf("sever input: ");
                fflush(stdout);
                std::cin >> buf;
                accept_socket.Send(buf);
            }
            accept_socket.Close();
            //如果不关闭，则会进入父进程的逻辑
            exit(0);
        }
        else
        {
            //父子进程各有一个accept_socket，父进程的accept没用，所以要关掉
            accept_socket.Close();
        }

    }
    listen_socket.Close();
    return 0;
}
