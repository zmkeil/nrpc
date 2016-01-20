#include <common.h>
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

    iobuf.print_info();
    iobuf.print_payload(2048);

    return 0;
}
