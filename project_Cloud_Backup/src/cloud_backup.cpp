#include"cloud_backup.hpp"

void test_compress(char* argv[])
{
    _cloud_sys::CompressUtil::Compress(argv[1],argv[2]);
    std::string tmp = argv[2];
    tmp += ".txt";
    _cloud_sys::CompressUtil::UnCompress(argv[2],tmp);
}

void test_storage()
{
    _cloud_sys::DataManager dm("./test.txt");
    //dm.Insert("a.txt","a.txt");
    //dm.Insert("b.txt","b.txt.gz");
    //dm.Insert("c.txt","c.txt.gz");
    //dm.Insert("d.txt","d.txt");
    //dm.Storage();
    std::vector<std::string> list;
    dm.InitLoad();
//    dm.Insert("a.txt","a.txt.gz");
    dm.GetAllFile(&list);
    for(const auto& e:list)
    {
        std::cout << e << std::endl;
    }
    std::cout << "-----------------" << std::endl;
    list.clear();
    dm.GetUnCompressList(&list);
    for(const auto& e:list)
    {
        std::cout << e << std::endl;
    }

}

int main(int argc, char* argv[])
{
    test_storage();
    return 0;
}
