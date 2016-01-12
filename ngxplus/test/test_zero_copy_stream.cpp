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
    // _bytes = IOBUF_MIN_PAYLOAD_SIZE
    iobuf.print_info();
    iobuf.print_payload();
    std::cout << std::endl << "-------------------------->" << std::endl;

	iobuf.read_point_cache();
    ngxplus::IOBufAsZeroCopyInputStream zero_copy_in(&iobuf);
    if (!zero_copy_in.Next((const void**)&buf_in, &size)) {
        std::cout << "in next error" << std::endl;
        return -1;
    }

    // _bytes = 0, _read_point == d.last
    iobuf.print_info();
    iobuf.print_payload();
    std::cout << std::endl << "-------------------------->" << std::endl;

	iobuf.read_point_resume();
	iobuf.print_info();
	iobuf.print_payload();
    return 0;
}
