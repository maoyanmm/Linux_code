#include"EpollServer.hpp"

#define CHECK_MY_FUN(f) if(f == false){return -1;}

int main()
{
    //首先创建监听套接字，并监听
    TcpSocket listen;
    CHECK_MY_FUN(listen.Socket());
    CHECK_MY_FUN(listen.Bind("192.168.132.128",4418));
    CHECK_MY_FUN(listen.Listen());

    //然后将监听套接字传入封装的epoll服务类
    EpollServer ep_svr; 
    ep_svr.AddFd(listen.GetFd(),false);
    
    //开始读出就绪的套接字
    while(1)
    {
        std::vector<TcpSocket> vec;
        if(ep_svr.WaitFd(&vec) == false)
        {
            continue;
        }
        for(size_t i = 0; i < vec.size(); ++i)
        {
            //如果就绪的套接字是监听套接字，则：
            //accept获取新的一对一服务的套接字new_fd
            if(vec[i].GetFd() == listen.GetFd())
            {
                printf("进入到了监听套接字\n");
                TcpSocket new_sock;
                std::string cli_ip;
                uint16_t cli_port;
                listen.Accept(&new_sock,&cli_ip,&cli_port);
                ep_svr.AddFd(new_sock.GetFd(),true);
                printf("new client connect!!!!!! %s:%d\n",cli_ip.c_str(),cli_port);
            }
            //如果不是监听的套接字，则是一对一服务的new_sock，则：
            //接受客户发送过来的数据
            else
            {
                printf("进入到了new_socket\n");
                int new_fd = vec[i].GetFd();
                vec[i].SetNonBlock();

                std::string buf;
                bool ret = vec[i].RecvNonBlock(&buf);

                //如果ret等于零表示接受数据错误，或者，客户端关闭了套接字，则
                //需要与这个客户对应的new_sock关闭掉，并且从epoll里删除
                if(ret == false)
                {
                    printf("客端关闭了\n");
                    close(new_fd);
                    ep_svr.DeleteFd(new_fd); 
                    continue;
                }
                printf("client said : %s\n",buf.c_str());

                std::string send_msg = "我是客户端";
                vec[i].SendNonBlock(send_msg);
            }
        }
        
    }


    return 0;
}
