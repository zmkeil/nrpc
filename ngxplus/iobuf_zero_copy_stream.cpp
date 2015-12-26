#include "iobuf_zero_copy_stream.h"

namespace ngxplus {


IOBufAsZeroCopyOutputStream::IOBufAsZeroCopyOutputStream(IOBuf* buf) : _buf(buf)
{
}

IOBufAsZeroCopyOutputStream::~IOBufAsZeroCopyOutputStream()
{
    _buf = nullptr;
}

bool IOBufAsZeroCopyOutputStream::Next(void** data, int* size)
{
    int ret = _buf->alloc((char**)data);
    if (ret == -1) {
        return false;
    }
    *size = ret;
    return true;
}

void IOBufAsZeroCopyOutputStream::BackUp(int count)
{
    _buf->reclaim(count);
}

int64_t IOBufAsZeroCopyOutputStream::ByteCount() const
{
    return (int64_t)_buf->get_byte_count();
}

}
