#include"FileNameManager.hpp"
#include<iostream>
using namespace std;
int main()
{
    FileNameManager::GetFNM()->Insert("test1.txt");
    FileNameManager::GetFNM()->Insert("test2.txt");
    FileNameManager::GetFNM()->Update("test2.txt",1);
    cout << "fnm.IsExist(test1.txt) = " <<  FileNameManager::GetFNM()->IsExist("test1.txt") << endl;
    cout << "fnm.IsExist(test2.txt) = " <<  FileNameManager::GetFNM()->IsExist("test2.txt") << endl;
    cout << "fnm.IsCompress(test1.txt) = " << FileNameManager::GetFNM()->IsCompress("test1.txt") << endl;
    cout << "fnm.IsCompress(test2.txt) = " << FileNameManager::GetFNM()->IsCompress("test2.txt") << endl << endl;
    vector<string> list; 
    FileNameManager::GetFNM()->GetAllFile(&list);
    cout << "fnm.GetAllFile()" << endl;
    for(const auto& file:list)
    {
        cout << file << endl;
    }

    vector<string> list2; 
    FileNameManager::GetFNM()->GetUnCompressList(&list2);
    cout << "fnm.GetUnCompressList()" << endl;
    for(const auto& file:list2)
    {
        cout << file << endl;
    }
    return 0;
}
