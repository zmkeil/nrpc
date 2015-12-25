
#include <string_printf.h>
#include "iobuf.h"
#include "log.h"
#include "info_log_context.h"

namespace ngxplus {

IOBuf::IOBuf() : _pool(nullptr), _size(IOBUF_DEFAULT_SIZE)
{
}

IOBuf::IOBuf(size_t size) : _pool(nullptr), _size(size)
{
    if (size < MIN_BLOCK_SIZE) {
        LOG(NGX_LOG_LEVEL_WARN, "pool's blocksize [%ld] too small, use MIN_BLCOK_SIZE [%ld]",
                size, MIN_BLOCK_SIZE);
        _size = MIN_BLOCK_SIZE;
    }
}

IOBuf::~IOBuf()
{
    ngx_destroy_pool(_pool);
}

// -1 : error
// postive num : alloced size
int IOBuf::alloc(char** buf, size_t size, IOBufAllocType type)
{
    if (!_pool) {
        _pool = ngx_create_pool(_size, NULL);
        if (!_pool) {
            LOG(NGX_LOG_LEVEL_ALERT, "create pool [size=%ld] failed", _size);
            return -1;
        }
        if (_pool->max < (MIN_BLOCK_SIZE - 2*sizeof(ngx_pool_t))) {
            LOG(NGX_LOG_LEVEL_ALERT, "pool's blocksize [%ld] too small", _size);
            return -1;
        }
    }

    if (size == 0) {
        LOG(NGX_LOG_LEVEL_WARN, "iobuf alloc size = 0");
        return -1;
    }

    if (size > _pool->max) {
        LOG(NGX_LOG_LEVEL_ALERT, "iobuf alloc size > pool.max,"
                "big_pool is not supported");
        return -1;
    }

    if (type == IOBUF_ALLOC_SIMILAR) {
        size = std::min((long)size, _pool->current->d.end - _pool->current->d.last);
    }
    LOG(NGX_LOG_LEVEL_NOTICE, "alloc size %ld", size);
    *buf = (char*)ngx_palloc(_pool, size);
    if (!(*buf)) {
        return -1;
    }

    // ensure data to be stored in series
    if (_pool->current->d.next) {
        _pool->current = _pool->current->d.next;
    }
    return size;
}

void IOBuf::print()
{
    return;
}

void IOBuf::dump_info(std::string* info)
{
    while (_pool)  
    {  
        common::string_appendf(info, "_pool = 0x%x\n", _pool);  
        common::string_appendf(info, "  .d\n");  
        common::string_appendf(info, "    .last = 0x%x\n", _pool->d.last);  
        common::string_appendf(info, "    .end = 0x%x\n", _pool->d.end);  
        common::string_appendf(info, "    .next = 0x%x\n", _pool->d.next);  
        common::string_appendf(info, "    .failed = %d\n", _pool->d.failed);  
        common::string_appendf(info, "  .max = %d\n", _pool->max);  
        common::string_appendf(info, "  .current = 0x%x\n", _pool->current);  
        common::string_appendf(info, "  .chain = 0x%x\n", _pool->chain);  
        common::string_appendf(info, "  .large = 0x%x\n", _pool->large);  
        common::string_appendf(info, "  .cleanup = 0x%x\n", _pool->cleanup);  
        common::string_appendf(info, "  .log = 0x%x\n", _pool->log);  
        common::string_appendf(info, "available _pool memory = %d\n\n", _pool->d.end - _pool->d.last);  
        _pool = _pool->d.next;  
    }
}

}
