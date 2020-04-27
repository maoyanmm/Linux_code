#include"ChatClient.hpp"

int main()
{
    ChatClient cc;
    cc.Register();
    cc.Login();
    while(1)
    {
        sleep(1);
    }
    return 0;
}
