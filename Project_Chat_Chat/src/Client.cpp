#include"ChatClient.hpp"

int main()
{
    ChatClient cc;
    cc.Register();
    cc.Login();
    while(1)
    {
        std::cout << "Please enter message:";
        fflush(stdout); 
        std::string msg;
        std::cin >> msg;
        cc.SendMsg(msg);
        cc.RecvMsg(&msg);
        std::cout << "Group said:" << msg << std::endl;
    }
    return 0;
}
