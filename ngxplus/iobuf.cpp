#include <common.h>
#include "iobuf.h"
#include "log.h"
#include "info_log_context.h"

namespace ngxplus {

IOBuf::IOBuf() : _pool(nullptr),
        _block_size(IOBUF_DEFAULT_BLOCK_SIZE),
        _blocks(0),
        _bytes(0),
        _read_block(0)
{
}

IOBuf::IOBuf(size_t size) : _pool(nullptr),
        _block_size(size),
        _blocks(0),
        _bytes(0),
        _read_block(0)
{
    if (size < (IOBUF_MIN_PAYLOAD_SIZE + 2 * sizeof(ngx_pool_t))) {
        LOG(NGX_LOG_LEVEL_WARN, "pool's blocksize [%ld] LT [%ld]",
                size, IOBUF_MIN_PAYLOAD_SIZE + 2 * sizeof(ngx_pool_t));
        _block_size = IOBUF_MIN_PAYLOAD_SIZE + 2 * sizeof(ngx_pool_t);
    }
}

IOBuf::~IOBuf()
{
    if (_pool) {
        ngx_destroy_pool(_pool);
    }
}

// -1 : error
// postive num : alloced size
int IOBuf::alloc(char** buf, size_t size, IOBufAllocType type)
{
    if (!_pool) {
        _pool = ngx_create_pool(_block_size, NULL);
        if (!_pool) {
            LOG(NGX_LOG_LEVEL_ALERT, "create pool [blocksize=%ld] failed", _block_size);
            return -1;
        }
        if (_pool->max < (IOBUF_MIN_PAYLOAD_SIZE)) {
            LOG(NGX_LOG_LEVEL_ALERT, "pool's blocksize [%ld] too small", _block_size);
            return -1;
        }
        _read_pool = _pool;
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

    long remain = _pool->current->d.end - _pool->current->d.last;
    if ((type == IOBUF_ALLOC_SIMILAR) && (remain > 0)) {
        size = std::min((long)size, remain);
    }
    //LOG(NGX_LOG_LEVEL_NOTICE, "alloc size %ld", size);
    // use ngx_pnalloc instead of ngx_palloc, ignore align
    *buf = (char*)ngx_pnalloc(_pool, size);
    if (!(*buf)) {
        return -1;
    }

    if (_blocks == 0) {
        _read_point = *buf;
        _start_points[_blocks++] = _read_point;
    }
    // ensure data to be stored in series
    if (_pool->current->d.next) {
        _pool->current = _pool->current->d.next;
        _start_points[_blocks++] = *buf;
        if (_blocks > IOBUF_MAX_BLOCKS_NUM) {
            LOG(NGX_LOG_LEVEL_WARN, "_blocks GT IOBUF_MAX_BLOCKS_NUM"
                    "try to increase _block_size [%ld]", _block_size);
            return -1;
        }
    }
    _bytes += size;
    return size;
}

int IOBuf::alloc(char** buf)
{
    return alloc(buf, IOBUF_MIN_PAYLOAD_SIZE, IOBUF_ALLOC_SIMILAR);
}

void IOBuf::reclaim(int count)
{
    // the count must GE current block's size
    long current_block_len = (char*)_pool->current->d.last - _start_points[_blocks - 1];
    if (current_block_len <= count) {
        LOG(NGX_LOG_LEVEL_ALERT, "reclaim count GE current_block_len");
        count = current_block_len - 1;
    }
    _pool->current->d.last -= count;
    _bytes -= count;
}

int IOBuf::read(const char** data)
{
    if ((!_read_pool) || (_bytes == 0)) {
        LOG(NGX_LOG_LEVEL_DEBUG, "no more data in zero_copy_input_stream");
        return -1;
    }

    // normally, _bytes > (last - read_point)
    // but when cutted, we can only read _bytes
    int remain = std::min((char*)_read_pool->d.last - _read_point, (long)_bytes);
    if (remain > 0) {
        *data = _read_point;
        _read_point += remain;
        _bytes -= remain;
        return remain;
    }

    _read_pool = _read_pool->d.next;
    if (!_read_pool) {
        LOG(NGX_LOG_LEVEL_DEBUG, "no more data in zero_copy_input_stream");
        return -1;
    }
    *data = _start_points[++_read_block];
    // maybe 0, no matter, read again
    remain = std::min((char*)_read_pool->d.last - *data, (long)_bytes);
    _read_point = (char*)*data + remain;
    _bytes -= remain;
    return remain;
}

void IOBuf::back(int count)
{
    int current_block_read_byte = _read_point - _start_points[_read_block];
    if (current_block_read_byte < count) {
        LOG(NGX_LOG_LEVEL_WARN, "back too much, only back this block");
        count = current_block_read_byte;
    }

    _read_point -= count;
    _bytes += count;
}

int IOBuf::skip(int count)
{
    int origin_count = count;
    while (count && _bytes) {
        int current_block_remain_byte = std::min((char*)_read_pool->d.last - _read_point, (long)_bytes);
        if (current_block_remain_byte < count) {
            count -= current_block_remain_byte;
            _bytes -= current_block_remain_byte;
            _read_point += current_block_remain_byte;
            if (_read_point == (char*)_read_pool->d.last)
            {
                _read_pool = _read_pool->d.next;
                if (!_read_pool) {
                    break;
                    return (origin_count - count);
                }
                _read_point = _start_points[_read_block++];
            }
        }
        else {
            _read_point += count;
            _bytes -= count;
            count = 0;
        }
    }
    if (count) {
        LOG(NGX_LOG_LEVEL_WARN, "skip too much, not enough data");
    }
    return origin_count - count;
}

void IOBuf::release_all()
{
    skip(_bytes);
}

size_t IOBuf::get_byte_count()
{
    return _bytes;
}

char* IOBuf::get_read_point()
{
    return _read_point;
}

bool IOBuf::cutn(int count)
{
    if ((long)count > (long)_bytes) {
        LOG(NGX_LOG_LEVEL_WARN, "not enough data to be cutted");
        _cut_remain_bytes = 0;
        //_bytes = _bytes;
        return false;
    }
    _cut_remain_bytes = _bytes - count;
    _bytes = count;
    return true;
}

void IOBuf::carrayon()
{
    _bytes += _cut_remain_bytes;
    _cut_remain_bytes = 0;
    return;
}

void IOBuf::dump_payload(std::string* payload)
{
    ngx_pool_t* p = _pool;
    int i;
    for (i = 0; i < _read_block; ++i) {
        p = p->d.next;
    }
    common::string_appendn(payload, _read_point, (size_t)((char*)p->d.last - _read_point));
    p = p->d.next;
    i++;
    while(p) {
        common::string_appendn(payload, _start_points[i],
                (size_t)((char*)p->d.last - _start_points[i]));
        i++;
        p = p->d.next;
    }
    return;
}

void IOBuf::print_payload(size_t capacity)
{
    std::string payload;
    payload.reserve(capacity);
    dump_payload(&payload);
    std::cout << payload << std::endl;
}

void IOBuf::print_payload()
{
    print_payload(IOBUF_DEFAULT_PAYLOAD_SIZE);
}

void IOBuf::dump_info(std::string* info)
{
    ngx_pool_t* p = _pool;
    int i = 0;
    while (p)
    {
        common::string_appendf(info, "p = %p\n", p);  
        common::string_appendf(info, "  .d\n");  
        common::string_appendf(info, "    .last = %p\n", p->d.last);  
        common::string_appendf(info, "    .end = %p\n", p->d.end);  
        common::string_appendf(info, "    .next = %p\n", p->d.next);  
        common::string_appendf(info, "    .failed = %ld\n", p->d.failed);  
        if (i == 0) {
            common::string_appendf(info, "  .max = %zu\n", p->max);  
            common::string_appendf(info, "  .current = %p\n", p->current);  
            common::string_appendf(info, "  .chain = %p\n", p->chain);  
            common::string_appendf(info, "  .large = %p\n", p->large);  
            common::string_appendf(info, "  .cleanup = %p\n", p->cleanup);  
            common::string_appendf(info, "  .log = %p\n", p->log);  
        }
        common::string_appendf(info, "available p memory = %ld\n", p->d.end - p->d.last);  
        common::string_appendf(info, "start_point = %p\n\n", _start_points[i++]);
        p = p->d.next;
    }
    common::string_appendf(info, "_read_point = %p\n", _read_point);
    common::string_appendf(info, "bytes = %zu\n", _bytes);
}

void IOBuf::print_info(size_t capacity)
{
    std::string info;
    info.reserve(capacity);
    dump_info(&info);
    std::cout << info << std::endl;
}

void IOBuf::print_info()
{
    print_info(IOBUF_DEFAULT_INFO_SIZE);
}

}
