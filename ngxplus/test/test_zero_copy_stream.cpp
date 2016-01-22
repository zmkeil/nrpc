#include <io/iobuf_zero_copy_stream.h>
#include "ngxplus_iobuf.h"

int main()
{

    char* buf;
    const char* buf_in;
    int size;


/* first alloc some data */
    ngxplus::NgxplusIOBuf iobuf(1024);
    common::IOBufAsZeroCopyOutputStream zero_copy_out(&iobuf);
    if (!zero_copy_out.Next((void**)&buf, &size)) {
        std::cout << "next error" << std::endl;
        return -1;
    }
    snprintf(buf, size, "hello\n");
    // _bytes = IOBUF_MIN_PAYLOAD_SIZE
    iobuf.print_info();
    iobuf.print_payload();
    std::cout << std::endl << "-------------------------->" << std::endl;

/* consume the data */
	iobuf.read_point_cache();
    common::IOBufAsZeroCopyInputStream zero_copy_in(&iobuf);
    if (!zero_copy_in.Next((const void**)&buf_in, &size)) {
        std::cout << "in next error" << std::endl;
        return -1;
    }
    // _bytes = 0, _read_point == d.last
    iobuf.print_info();
    iobuf.print_payload();
    std::cout << std::endl << "-------------------------->" << std::endl;

/* again alloc some data */
    if (!zero_copy_out.Next((void**)&buf, &size)) {
        std::cout << "next error" << std::endl;
        return -1;
    }
    snprintf(buf, size, "again world\n");
    // _bytes = IOBUF_MIN_PAYLOAD_SIZE
    iobuf.print_info();
    iobuf.print_payload();
    std::cout << std::endl << "-------------------------->" << std::endl;

/* resume the state after first alloc */
	iobuf.read_point_resume();
	iobuf.print_info();
	iobuf.print_payload();
    return 0;
}
