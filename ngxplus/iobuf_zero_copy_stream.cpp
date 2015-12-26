#include "iobuf_zero_copy_stream.h"

namespace ngxplus {

// InputStream
IOBufAsZeroCopyInputStream::IOBufAsZeroCopyInputStream(IOBuf* buf) : _buf(buf)
{
}

IOBufAsZeroCopyInputStream::~IOBufAsZeroCopyInputStream()
{
    _buf = nullptr;
}

bool IOBufAsZeroCopyInputStream::Next(const void** data, int* size)
{
    int ret = _buf->read((const char**)data);
    if (ret > 0) {
        *size = ret;
        return true;
    }

    return false;
}

void IOBufAsZeroCopyInputStream::BackUp(int count)
{
    _buf->back(count);
}

bool IOBufAsZeroCopyInputStream::Skip(int count)
{
    int ret = _buf->skip((size_t)count);
    return (ret == count) ? true : false;
}

int64_t IOBufAsZeroCopyInputStream::ByteCount() const
{
    return (int64_t)_buf->get_byte_count();
}


// OutputStream
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
