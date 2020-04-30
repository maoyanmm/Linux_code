#include"ChatClient.hpp"
#include"ChatWindow.hpp"

int main()
{
    ChatClient cc;
    cc.Register();
    cc.Login();
    ChatWindow cw;
    while(1)
    {
        cw.Start(&cc); 
       //std::cout << "Enter message：";
       //fflush_unlocked(stdout);
       //std::string msg;
       //std::cin >> msg;
       //Json::Value val;
       //val["_nick_name"] = "石帅";
       //val["_school"] = "西安工业大学";
       //val["_msg"] = msg;
       //val["_user_id"] = 0;
       //Json::FastWriter writer;    
       //std::string send_msg = writer.write(val);
       //cc.SendMsg(send_msg);
       //std::string recv_msg;
       //cc.ReceiveMsg(&recv_msg);
       //std::cout << "收到消息-----------" << recv_msg << std::endl;
    }
    return 0;
}
