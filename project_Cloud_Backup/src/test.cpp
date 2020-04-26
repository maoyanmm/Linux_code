#include"cloud_backup.hpp"

int main()
{
    _cloud_sys::CompressUtil::Compress("old1.txt","old1.txt.gz");
    _cloud_sys::CompressUtil::UnCompress("old1.txt.gz","new1.txt");
    return 0;
}
