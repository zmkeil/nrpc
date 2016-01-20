#ifndef NGXPLUS_IOBUF_ZERO_COPY_BUF_H
#define NGXPLUS_IOBUF_ZERO_COPY_BUF_H

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/common.h>
#include <common_head.h>
#include "iobuf.h"

namespace ngxplus {

class IOBufAsZeroCopyInputStream :  public google::protobuf::io::ZeroCopyInputStream
{
public:
    IOBufAsZeroCopyInputStream(IOBuf* buf);
    virtual ~IOBufAsZeroCopyInputStream();

    // implements ZeroCopyInputStream ----------------------------------
    bool Next(const void** data, int* size);
    void BackUp(int count);
    bool Skip(int count);
    int64_t ByteCount() const;

private:
    IOBuf* _buf;
};

class IOBufAsZeroCopyOutputStream : public google::protobuf::io::ZeroCopyOutputStream
{
public:
    IOBufAsZeroCopyOutputStream(IOBuf* buf);
    virtual ~IOBufAsZeroCopyOutputStream();

    // implements ZeroCopyOutputStream ----------------------------------
    bool Next(void** data, int* size);
    void BackUp(int count);
    int64_t ByteCount() const;

private:
    IOBuf* _buf;
};

}
#endif
