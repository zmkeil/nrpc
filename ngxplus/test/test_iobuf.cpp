#include "iobuf.h"

inline void info_string(std::string& str)
{
    printf("capacity:%ld, size:%ld, content:\"%s\"\n", str.capacity(), str.size(), str.c_str());
}

int main()
{
    ngxplus::IOBuf iobuf(768);
    char* buf;

    ssize_t size = iobuf.alloc(&buf, 300, ngxplus::IOBuf::IOBUF_ALLOC_EXACT);
    std::cout << "palloc size: " << size << std::endl;
    memset(buf, 'a', size);

    size = iobuf.alloc(&buf, 512, ngxplus::IOBuf::IOBUF_ALLOC_EXACT);
    std::cout << "palloc size: " << size << std::endl;
    memset(buf, 'b', size);

    size = iobuf.alloc(&buf, 512, ngxplus::IOBuf::IOBUF_ALLOC_SIMILAR);
    std::cout << "palloc size: " << size << std::endl;
    memset(buf, 'c', size);

    std::string info;
    info.reserve(1024);
    iobuf.dump_info(&info);
    std::cout << info << std::endl;

    std::string payload;
    payload.reserve(2048);
    iobuf.dump_payload(&payload);
    std::cout << payload << std::endl;
 
    return 0;
}
