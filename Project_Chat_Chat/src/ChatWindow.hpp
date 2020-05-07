#pragma once
#include<pthread.h>
#include<ncurses.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdlib.h>

#include"FormatMessage.hpp"
#include"ChatClient.hpp"
#include"Log.hpp"

class ChatWindow;
class Helper
{
    public:
        ChatWindow* _cw;
        ChatClient* _cc;
        int _thread_num;
    public:
        Helper(ChatWindow* cw,ChatClient* cc,int thread_num)
            :_cw(cw),_cc(cc),_thread_num(thread_num)
        {

        }
};
class ChatWindow
{
    private:
        //输出顶部的标识语
        WINDOW* _head;
        //输出所有的聊天信息
        WINDOW* _output;
        //输入框
        WINDOW* _input;
        //输出所有在线用户
        WINDOW* _user_list;
        //窗口刷新的时候是线程不安全的，需要加锁
        pthread_mutex_t _lock;
        ChatClient* _cc;
    public:
        ChatWindow(ChatClient* cc)
            :_head(nullptr),_output(nullptr),_input(nullptr),_user_list(nullptr),_cc(cc)
        {
            pthread_mutex_init(&_lock,NULL);
            //初始化屏幕
            initscr();
            //0表示不显示光标
            curs_set(0);
        }
        ~ChatWindow()
        {
            if(_head)
                delwin(_head);
            if(_output)
                delwin(_output);
            if(_input)
                delwin(_input);
            if(_user_list)
                delwin(_user_list);
            //销毁屏幕变量
            endwin();
            pthread_mutex_destroy(&_lock);
        }
        void Start()
        {
            std::vector<pthread_t> threads;
            pthread_t tid;
            for(int i = 0; i < 4; ++i)
            {
                Helper* hp = new Helper(this,_cc,i);
                int ret = pthread_create(&tid,NULL,DrawWindowStart,(void*)hp);
                if(ret < 0)
                {
                    LOG(FATAL,"DrawWindowStart failed!") << std::endl;
                    exit(1);
                }
                threads.push_back(tid);
            }
            for(int i = 0; i < 4; ++i)
            {
                pthread_join(threads[i],NULL);
            }
        }
    private:
        static void* DrawWindowStart(void* arg)
        {
           Helper* hp = (Helper*)arg;
           int thread_num = hp->_thread_num;
           ChatClient* cc = hp->_cc;
           ChatWindow* cw = hp->_cw;
           switch(thread_num)
           {
               case 0:
                   RunHead(cw);
                   break;
               case 1:
                   RunOutPut(cw,cc);
                   break;
               case 2:
                   RunInPut(cw,cc);
                   break;
               case 3:
                   RunUserList(cw,cc);
                   break;
           }
           return NULL;
        }
        static void RunHead(ChatWindow* cw)
        {
            std::string tips = "Chat Chat ~ ~";
            int y,x;
            size_t pos = 1;
            char direction = 'r';
            while(1)
            {
                getmaxyx(cw->_head,y,x);
                cw->DrawHead();
                cw->PutStringToWindow(cw->_head,y/2,pos,tips);
                if(pos > x-tips.size()-2)
                {
                    direction = 'l';
                }
                if(pos < 2)
                {
                    direction = 'r';
                }
                if(direction == 'l')
                {
                    --pos;
                }
                else
                {
                    ++pos;
                }
                sleep(1);
            }
        }
        static void RunOutPut(ChatWindow* cw,ChatClient* cc)
        {
            std::string recv_msg;
            FormatMessage fmsg; 
            cw->DrawOutput();
            int y,x;
            int row = 1;
            while(1)
            {
                getmaxyx(cw->_output,y,x);
                //1、接收消息并解析消息
                cc->RecvMsg(&recv_msg);
                fmsg.Deserialize(recv_msg);
                //2、把消息内容打印在屏幕
                std::string show_msg = fmsg._nick_name + " : " + fmsg._msg;
                cw->PutStringToWindow(cw->_output,row,1,show_msg);
                //3、把这条消息的用户信息存起来
                std::string user_info = fmsg._birthday + " - " + fmsg._nick_name;
                cc->PushOnlineUser(user_info);
                ++row;
                if(row > y - 2)
                {
                    row = 1;
                    cw->DrawOutput();
                }
            }
        }
        static void RunInPut(ChatWindow* cw,ChatClient* cc)
        {
            std::string tips = "Enter Message : ";
            std::string send_msg; 
            while(1)
            {
                cw->DrawInput();
                cw->PutStringToWindow(cw->_input,1,1,tips);
                cw->GetStringFromWindow(cw->_input,&send_msg);
                cc->SendMsg(send_msg);
            }
        }
        static void RunUserList(ChatWindow* cw,ChatClient* cc)
        {
            cw->DrawUserList();
            while(1)
            {
                cw->DrawUserList();
                auto user_list = cc->GetOnlineUser();
                int row = 1;
                for(const auto& user_info:user_list)
                {
                    cw->PutStringToWindow(cw->_user_list,row++,1,user_info);
                }
                sleep(1);
            }
        }
    private:
        void PutStringToWindow(WINDOW* win,int y,int x,const std::string& msg)
        {
            mvwaddstr(win,y,x,msg.c_str());
            pthread_mutex_lock(&_lock);
            wrefresh(win);
            pthread_mutex_unlock(&_lock);
        }
        void GetStringFromWindow(WINDOW* win,std::string* msg)
        {
            char buf[1024] = {'\0'};
            memset(buf,'\0',sizeof(buf));
            wgetnstr(win,buf,sizeof(buf)-1);
            msg->assign(buf,strlen(buf));
        }
        void DrawHead()
        {
            int row = LINES/5;
            int col = COLS;
            int start_y = 0;
            int start_x = 0;
            //初始化：从(x,y)开始绘制：|=row,一=col的框
            _head = newwin(row,col,start_y,start_x);
            //两个0表示：横竖的边框都采用默认的符号
            box(_head,0,0); 
            //一定要加锁，不加锁会导致所有模块的框框会混乱
            pthread_mutex_lock(&_lock);
            wrefresh(_head);
            pthread_mutex_unlock(&_lock);
        }
        void DrawOutput()
        {
            int row = (LINES*3)/5;
            int col = (COLS*3)/4;
            int start_y = LINES/5;
            int start_x = 0;
            //初始化：从(x,y)开始绘制：|=row,一=col的框
            _output = newwin(row,col,start_y,start_x);
            //两个0表示：横竖的边框都采用默认的符号
            box(_output,0,0); 
            //一定要加锁，不加锁会导致所有模块的框框会混乱
            pthread_mutex_lock(&_lock);
            wrefresh(_output);
            pthread_mutex_unlock(&_lock);
        }
        void DrawInput()
        {
            int row = LINES/5;
            int col = (COLS*3)/4;
            int start_y = (LINES*4)/5;
            int start_x = 0;
            //初始化：从(x,y)开始绘制：|=row,一=col的框
            _input = newwin(row,col,start_y,start_x);
            //两个0表示：横竖的边框都采用默认的符号
            box(_input,0,0); 
            //一定要加锁，不加锁会导致所有模块的框框会混乱
            pthread_mutex_lock(&_lock);
            wrefresh(_input);
            pthread_mutex_unlock(&_lock);
        }
        void DrawUserList()
        {
            int row = (LINES*4)/5;
            int col = (COLS*1)/4;
            int start_y = LINES/5;
            int start_x = (COLS*3)/4;
            //初始化：从(x,y)开始绘制：|=row,一=col的框
            _user_list = newwin(row,col,start_y,start_x);
            //两个0表示：横竖的边框都采用默认的符号
            box(_user_list,0,0); 
            //一定要加锁，不加锁会导致所有模块的框框会混乱
            pthread_mutex_lock(&_lock);
            wrefresh(_user_list);
            pthread_mutex_unlock(&_lock);
        }
};
