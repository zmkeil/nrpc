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

    // return alloc size
    int alloc(char** buf, size_t size, IOBufAllocType type);
    // MIN_PAYLOAD SIMILAR alloc
    int alloc(char** buf);

    void reclaim(int count);

    // return read size
    int read(const char** data);

    int skip(int count);

    void back(int count);

    size_t get_byte_count();

    // cutn and carrayon must appear in pairs and continously
    bool cutn(int count);
    void carrayon();

    void dump_payload(std::string* payload);
    void print_payload();
    void dump_info(std::string* info);
    void print_info();

private:
    static const size_t DEFAULT_BLOCK_SIZE = 1024;
    static const size_t MIN_PAYLOAD_SIZE = 200/* _block_size - 2*sizeof(ngx_pool_t) */;
    static const size_t MAX_BLOCKS_NUM = 10;

private:
    ngx_pool_t* _pool;
    size_t _block_size;
    size_t _blocks/*LE MAX_BLOCKS_NUM*/;
    size_t _bytes;
    size_t _cut_remain_bytes;
    char* _start_points[MAX_BLOCKS_NUM];
    char* _read_point;
    int _read_block;
    ngx_pool_t* _read_pool;
};

}
#endif
