#ifndef NGXPLUS_IOBUF_H
#define NGXPLUS_IOBUF_H

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_log.h>
}
#include "common.h"

namespace ngxplus {
 
enum IOBufAllocType {
    IOBUF_ALLOC_EXACT = 0,
    IOBUF_ALLOC_SIMILAR
};

class IOBuf
{
public:
    IOBuf();
    IOBuf(size_t size);
    virtual ~IOBuf();
    
    int alloc(char** buf, size_t size, IOBufAllocType type);

    void print();

    void dump_info(std::string* info);

private:
    ngx_pool_t* _pool;
    size_t _size;

private:
    static const size_t IOBUF_DEFAULT_SIZE = 1024;
    static const size_t MIN_BLOCK_SIZE = 200;
};

}
#endif
