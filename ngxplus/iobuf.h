#ifndef NGXPLUS_IOBUF_H
#define NGXPLUS_IOBUF_H

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_log.h>
}
#include "common.h"

namespace ngxplus {
 
class IOBuf
{
public:
    enum IOBufAllocType {
        IOBUF_ALLOC_EXACT = 0,
        IOBUF_ALLOC_SIMILAR
    };

public:
    IOBuf();
    IOBuf(size_t size);
    virtual ~IOBuf();
    
    int alloc(char** buf, size_t size, IOBufAllocType type);
    // MIN_PAYLOAD SIMILAR alloc
    int alloc(char** buf);

    void reclaim(int count);

    size_t get_byte_count();

    void dump_payload(std::string* payload);

    void dump_info(std::string* info);

private:
    static const size_t DEFAULT_BLOCK_SIZE = 1024;
    static const size_t MIN_PAYLOAD_SIZE = 200/* _block_size - 2*sizeof(ngx_pool_t) */;
    static const size_t MAX_BLOCKS_NUM = 10;

private:
    ngx_pool_t* _pool;
    size_t _block_size;
    size_t _blocks/*LE MAX_BLOCKS_NUM*/;
    size_t _bytes;
    char* _start_points[MAX_BLOCKS_NUM];
    char* _read_point;
};

}
#endif
