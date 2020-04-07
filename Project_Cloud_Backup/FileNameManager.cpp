#include"FileNameManager.hpp"
#include<iostream>
using namespace std;
int main()
{
    MYSQL* mysql = MysqlInit();
    if(mysql == NULL)
    {
        return -1;
    }
    FileNameManager fnm(mysql);
    fnm.Insert("test1.txt");
    fnm.Insert("test2.txt");
    fnm.Update("test2.txt",1);
    cout << "fnm.IsExist(test1.txt) = " <<  fnm.IsExist("test1.txt") << endl;
    cout << "fnm.IsExist(test2.txt) = " <<  fnm.IsExist("test2.txt") << endl;
    cout << "fnm.IsCompress(test1.txt) = " << fnm.IsCompress("test1.txt") << endl;
    cout << "fnm.IsCompress(test2.txt) = " << fnm.IsCompress("test2.txt") << endl << endl;
    vector<string> list; 
    fnm.GetAllFile(&list);
    cout << "fnm.GetAllFile()" << endl;
    for(const auto& file:list)
    {
        cout << file << endl;
    }

    vector<string> list2; 
    fnm.GetUnCompressList(&list2);
    cout << "fnm.GetUnCompressList()" << endl;
    for(const auto& file:list2)
    {
        cout << file << endl;
    }


    MysqlDestroy(mysql);
    return 0;
}
