#pragma once
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<vector>
#include<stdio.h>
#include<ncurses.h>

#include"ChatClient.hpp"

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
           }
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
                   cw->DrawHeader();
                   break;
               case 1:
                   cw->DrawOutPut();
                   break;
               case 2:
                   cw->DrawUserList();
                   break;
               case 3:
                   cw->DrawInput();
                   break;
               default:
                   break;
           }
           delete hp;
           return NULL;
       }
}
;
