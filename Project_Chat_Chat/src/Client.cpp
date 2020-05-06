#include"ChatWindow.hpp"

void Menu()
{
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "|  1.register                      2.login  |" << std::endl;
    std::cout << "|                  3.exit                   |" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << std::endl;
}

int main()
{
    ChatClient* cc = new ChatClient();
    while(1)
    {
        Menu();
        int select = -1;
        std::cout << "Please select : ";
        fflush(stdout);
        std::cin >> select;
        if(select == 1)
        {
            if(cc->Register())
            {
                std::cout << "Register success! Please login!" << std::endl;
            }
            else
            {
                std::cout << "Register failed! Please check your register info!" << std::endl;
            }
        }
        else if(select == 2)
        {
            if(!cc->Login())
            {
                std::cout << "Login failed! Please check your login info!" << std::endl;
            }
            ChatWindow cw(cc);
            cw.Start();
        }
        else if(select == 3)
        {
            return 0;
        }
        else
        {
            std::cout << "Select error!" << std::endl;
            sleep(2);
        }
    }
    delete cc;
    return 0;
}
