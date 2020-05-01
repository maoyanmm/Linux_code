#pragma once
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<vector>
#include<stdio.h>
#include<string>
#include<ncurses.h>
#include<sstream>

#include"ChatClient.hpp"
#include"Message.hpp"

class ChatWindow;
class Helper
{
    public:
        ChatWindow* _chat_window;
        int _thread_num;
        ChatClient* _chat_cli;
    public:
        Helper(ChatWindow* cw,int thread_num,ChatClient* cc)
        {
            _chat_window = cw;
            _thread_num = thread_num;
            _chat_cli = cc;
        }
};

class ChatWindow
{
    private:
        WINDOW* _header;
        WINDOW* _output;
        WINDOW* _user_list;
        WINDOW* _input;
        std::vector<pthread_t> _threads;
        pthread_mutex_t _lock;
    public:
        ChatWindow()
        {
            _header = NULL;
            _output = NULL;
            _user_list = NULL;
            _input = NULL;
            _threads.clear();
            pthread_mutex_init(&_lock,NULL);
            initscr();
            //0为不显示光标
            curs_set(0);
        }
        ~ChatWindow()
        {
            if(_header)
                delwin(_header);
            if(_output)
                delwin(_output);
            if(_user_list)
                delwin(_user_list);
            if(_input)
                delwin(_input);
            endwin();
            pthread_mutex_destroy(&_lock);
        }
        void DrawHeader()
        {
            int lines = LINES/5;
            int cols = COLS;
            int start_x = 0;
            int start_y = 0;
            _header = newwin(lines,cols,start_y,start_x);
            box(_header,0,0);
            pthread_mutex_lock(&_lock);
            wrefresh(_header);
            pthread_mutex_unlock(&_lock);
        }
        void DrawOutPut()
        {
            int lines = (LINES*3)/5;
            int cols = (COLS*3)/4;
            int start_x = 0;
            int start_y = LINES/5;
            _output = newwin(lines,cols,start_y,start_x);
            box(_output,0,0);
            pthread_mutex_lock(&_lock);
            wrefresh(_output);
            pthread_mutex_unlock(&_lock);
        }
        void DrawUserList()
        {
            int lines = (LINES*3)/5;
            int cols = COLS/4;
            int start_x = (COLS*3)/4;
            int start_y = LINES/5;
            _user_list = newwin(lines,cols,start_y,start_x);
            box(_user_list,0,0);
            pthread_mutex_lock(&_lock);
            wrefresh(_user_list);
            pthread_mutex_unlock(&_lock);
        }
        void DrawInput()
        {
            int lines = LINES/5;
            int cols = COLS;
            int start_x = 0;
            int start_y = (LINES*4)/5;
            _input = newwin(lines,cols,start_y,start_x);
            box(_input,0,0);
            pthread_mutex_lock(&_lock);
            wrefresh(_input);
            pthread_mutex_unlock(&_lock);
        }
        
       void Start(ChatClient* cc)
       {
           pthread_t tid;
           for(int i = 0; i < 4; ++i)
           {
               Helper* hp = new Helper(this,i,cc);
               int ret = pthread_create(&tid,NULL,DrawWindowStart,(void*)hp);
               if(ret < 0)
               {
                   printf("Create thread failed!\n");
                   exit(1);
               }
               _threads.push_back(tid);
           }
           for(int i = 0; i < 4; ++i)
           {
               pthread_join(_threads[i],NULL);
           }
       }
       void RunHeader()
       {
           int x,y;
           size_t pos = 1;
           std::string welcome = "Welcome to chat chat";
           int flag = 0;
           while(1)
           {
               DrawHeader();
               getmaxyx(_header,y,x);
               PutStringToWindow(_header,y/2,pos,welcome); 
               //判断welcome要往哪移动
               if(pos < 2)
               {
                   flag = 0;
               }
               else if(pos > x-welcome.size()-2)
               {
                    flag = 1;
               }
               //flag = 0就向右移动
               if(flag == 0)
               {
                   ++pos;
               }
               //flag = 1就向左移动
               else
               {
                   --pos;
               }
               sleep(1);
           }

       }
       void RunInput(ChatClient* cc)
       {
           Message msg;
           msg._nick_name = cc->GetMyInfo()._nick_name;
           msg._school = cc->GetMyInfo()._school;
           msg._user_id = cc->GetMyInfo()._user_id;
           //输入在对话框的信息
           std::string enter_msg;
           //最后要发送的数据：包括对话框输入的信息、用户信息
           std::string send_mgs;
           std::string tips = "Please Enter:";
           while(1)
           {
               DrawInput();
               PutStringToWindow(_input,2,2,tips);
               GetStringFromWindow(_input,&enter_msg);
               msg._msg = enter_msg;
               msg.Serialize(&send_mgs);
               cc->SendMsg(send_mgs);
           }
       }
       void RunUserList(ChatClient* cc)
       {
           int y,x;
           getmaxyx(_user_list,y,x);
           while(1)
           {
               DrawUserList();
               auto user_list = cc->GetOnlineUser();             
               int row = 1;
               for(auto& user:user_list)
               {
                   PutStringToWindow(_user_list,row++,1,user);
               }
               sleep(1);
           }
       }
       void RunOutPut(ChatClient* cc)
       {
           std::string recv_msg;
           Message msg;
           DrawOutPut();
           int row = 1;
           int x,y;
           while(1)
           {
               getmaxyx(_output,y,x);
               cc->ReceiveMsg(&recv_msg);
               msg.Deserialize(recv_msg); 
               std::string show_msg;
               show_msg += msg._nick_name + "-" + msg._school + " : " + msg._msg;
               PutStringToWindow(_output,row,1,show_msg);
               //收到一个消息，就把这个人的信息放到在线列表里
               std::string user_info;
               std::stringstream ss;            
               ss << msg._user_id;
               std::string user_id;
               ss >> user_id;
               user_info += user_id + "-" + msg._school + "-" +msg._nick_name;
               cc->PushOnlineUser(user_info);
               //将消息往下放
               ++row;
               if(row > y - 2)
               {
                   row = 1;
                   DrawOutPut();
               }
               sleep(1);
           }
       }
       void PutStringToWindow(WINDOW* win,int row,int col,const std::string& msg)
       {
            mvwaddstr(win,row,col,msg.c_str());
            pthread_mutex_lock(&_lock);
            wrefresh(win);
            pthread_mutex_unlock(&_lock);
       }
       void GetStringFromWindow(WINDOW* win,std::string* data)
       {
           char buf[1024] = {0};
           memset(buf,'\0',sizeof(buf));
           wgetnstr(win,buf,sizeof(buf)-1);
           data->assign(buf,strlen(buf));
       }
   private:
       static void* DrawWindowStart(void* arg)
       {
           Helper* hp = (Helper*)arg;
           ChatWindow* cw = hp->_chat_window;
           int thread_num = hp->_thread_num;
           ChatClient* cc = hp->_chat_cli;
           switch(thread_num)
           {
               case 0:
                   cw->RunHeader();
                   break;
               case 1:
                   cw->RunOutPut(cc);
                   break;
               case 2:
                   cw->RunUserList(cc);
                   break;
               case 3:
                   cw->RunInput(cc);
                   break;
               default:
                   break;
           }
           delete hp;
           return NULL;
       }
}
;
