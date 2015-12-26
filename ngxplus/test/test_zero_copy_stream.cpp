#include "iobuf_zero_copy_stream.h"

int main()
{
    ngxplus::IOBuf iobuf;
    ngxplus::IOBufAsZeroCopyOutputStream zero_copy_out(&iobuf);

    char* buf;
    int size;
    if (!zero_copy_out.Next((void**)&buf, &size)) {
        std::cout << "next error" << std::endl;
        return -1;
    }

    snprintf(buf, size, "hello\n");

    std::string info;
    info.reserve(1024);
    iobuf.dump_info(&info);
    std::cout << info << std::endl;

    std::string payload;
    payload.reserve(1024);
    iobuf.dump_payload(&payload);
    std::cout << payload << std::endl;
    return 0;
}
