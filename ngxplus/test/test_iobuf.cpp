#include "iobuf.h"

inline void info_string(std::string& str)
{
    printf("capacity:%ld, size:%ld, content:\"%s\"\n", str.capacity(), str.size(), str.c_str());
}

int main()
{
    ngxplus::IOBuf iobuf(1024);
    char* buf;

    ssize_t size = iobuf.alloc(&buf, 10, ngxplus::IOBUF_ALLOC_EXACT);
    std::cout << "palloc size: " << size << std::endl;

    size = iobuf.alloc(&buf, 512, ngxplus::IOBUF_ALLOC_EXACT);
    std::cout << "palloc size: " << size << std::endl;

    size = iobuf.alloc(&buf, 512, ngxplus::IOBUF_ALLOC_SIMILAR);
    std::cout << "palloc size: " << size << std::endl;

    std::string info;
    info_string(info);
    info.reserve(1024);
    info_string(info);
    iobuf.dump_info(&info);

    return 0;
}
