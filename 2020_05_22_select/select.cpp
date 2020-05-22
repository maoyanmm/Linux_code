#include"SelectServer.hpp"

int main()
{
    //1、先创建监听的套接字
    TcpSocket listen;
    bool ret = listen.Socket();
    if(ret == false)
    {
        return -1;
    }
    ret = listen.Bind("192.168.132.128",19999);
    if(ret == false)
    {
        return -1;
    }
    ret = listen.Listen();
    if(ret == false)
    {
        return -1;
    }
    
    //2、然后将监听的套接字设置到select的fd_set里
    SelectServer ss;
    ss.AddFd(listen.GetFd());
    
    //3、调用select去监听
    while(1)
    {
        //创建一个装：已经读就绪的套接字，的数组
        std::vector<TcpSocket> ready;
        //如果本次select超时，或者失败，则跳过这一次
        if(ss.WaitFd(&ready) == false)
        {
            continue;
        }
        //遍历就绪套接字的数组
        for(size_t i = 0; i < ready.size(); ++i)
        {
            //如果就绪的套接字是listen的监听套接字，则执行逻辑是：将为客户一对一服务的new_fd添加到select的等待fd里
            if(listen.GetFd() == ready[i].GetFd())
            {
                TcpSocket new_fd;
                std::string peer_ip;
                uint16_t peer_port;
                //这里不会阻塞，因为select已经告诉我们listen已经监听到了有新的连接到来了
                listen.Accept(&new_fd,&peer_ip,&peer_port);
                std::cout << "have a new client connect : " << peer_ip << " : " << peer_port; 
                ss.AddFd(new_fd.GetFd());
            }
            //如果就绪的套接字是new_fd，也就是负责和客户端交互的fd，则执行的逻辑是：接收客户发来的数据
            else
            {
                TcpSocket new_fd = ready[i];
                std::string message;
                if(new_fd.Recv(message) == false)
                {
                    continue;
                }
                
                std::cout << "Client said : " << message << std::endl;
            }
        }
    }


    return 0;
}
