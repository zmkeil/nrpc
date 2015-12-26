#include "iobuf_zero_copy_stream.h"

int main()
{
    ngxplus::IOBuf iobuf;
    ngxplus::IOBufAsZeroCopyOutputStream zero_copy_out(&iobuf);

    char* buf;
    const char* buf_in;
    int size;
    if (!zero_copy_out.Next((void**)&buf, &size)) {
        std::cout << "next error" << std::endl;
        return -1;
    }

    snprintf(buf, size, "hello\n");

    ngxplus::IOBufAsZeroCopyInputStream zero_copy_in(&iobuf);
    if (!zero_copy_in.Next((const void**)&buf_in, &size)) {
        std::cout << "in next error" << std::endl;
        return -1;
    }

    std::string info;
    info.reserve(1024);
    iobuf.dump_info(&info);
    std::cout << info << std::endl;

    std::string payload;
    payload.reserve(1024);
    iobuf.dump_payload(&payload);
    std::cout << "size: " << zero_copy_out.ByteCount() << ", payload: " << payload << std::endl;
    return 0;
}
