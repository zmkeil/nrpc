#include <iostream>
#include <string_printf.h>
extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_log.h>
}
#include "ngxplus_iobuf.h"

namespace ngxplus {

bool NgxplusIOBuf::init_pool()
{
	_pool = ngx_create_pool(_block_size + 2 * sizeof(ngx_pool_t), NULL);
	if (!_pool) {
		LOG(ALERT, "create pool [blocksize=%lu] failed", _block_size);
		return -1;
	}
	if (_pool->max < _block_size) {
		LOG(ALERT, "pool.max [%lu] LT block_size [%lu]", _pool->max, _block_size);
		return -1;
	}

    _read_point = (char*)ngx_pnalloc(_pool, 0);
    _blocks = 1;

    _start_points[_blocks - 1] = _read_point;
    return true;
}

bool NgxplusIOBuf::alloc_next_block()
{
    char* buf;
    // use ngx_pnalloc instead of ngx_palloc, ignore align
    buf = (char*)ngx_pnalloc(_pool, _block_size);
    if (!buf) {
        LOG(WARN, "ngx_pnalloc [blocks=%lu] failed", _blocks);
        return false;
    }

    // ensure data to be stored in series
    if (_pool->current->d.next) {
        _pool->current = _pool->current->d.next;
    } else {
        LOG(ALERT, "alloc in the current block when alloc_next_block");
        return false;
    }

    _blocks++;
    if (_blocks > IOBUF_MAX_BLOCKS_NUM) {
        LOG(WARN, "_blocks GT IOBUF_MAX_BLOCKS_NUM"
                "try to increase _block_size [%lu]", _block_size);
        return false;
    }

    _start_points[_blocks - 1] = buf;

    reclaim_to_current_block(_block_size);
    return true;
}

bool NgxplusIOBuf::is_current_block_buf_enough(size_t size, common::IOBufAllocType type)
{
    size_t remain_buf_size = current_block_remain_buf_size();
    if ((type == common::IOBUF_ALLOC_SIMILAR) && (remain_buf_size > 0)) {
        return true;
    }
    else if ((type == common::IOBUF_ALLOC_EXACT) && (remain_buf_size >= size)) {
        return true;
    }
    return false;
}

size_t NgxplusIOBuf::current_block_remain_buf_size()
{
    return (size_t)(_pool->current->d.end - _pool->current->d.last);
}

size_t NgxplusIOBuf::current_block_alloc_buf_size()
{
    return (size_t)((char*)_pool->current->d.last - _start_points[_blocks - 1]);
}

char* NgxplusIOBuf::alloc_from_current_block(size_t n)
{
    return (char*)ngx_pnalloc(_pool, n);
}

bool NgxplusIOBuf::reclaim_to_current_block(size_t n)
{
    _pool->current->d.last -= n;
    return true;
}

void NgxplusIOBuf::move_read_point_to_next_block()
{
    _read_point = _start_points[++_read_block];
}

static ngx_pool_t* get_current_read_pool(ngx_pool_t* root, int read_block)
{
    ngx_pool_t* pool = root;
    while(read_block > 0) {
        pool = pool->d.next;
        if (!pool) {
            LOG(ALERT, "_read_block is GT actual count of blocks");
            return NULL;
        }
        read_block--;
    }
    return pool;
}

size_t NgxplusIOBuf::current_block_remain_data_size()
{
    ngx_pool_t* current_read_pool = get_current_read_pool(_pool, _read_block);
    if (!current_read_pool) {
        return 0;
    }
    return (size_t)((char*)current_read_pool->d.last - _read_point);
}

size_t NgxplusIOBuf::current_block_consume_data_size()
{
    return (size_t)(_read_point - _start_points[_read_block]);
}


// debugs

void NgxplusIOBuf::dump_payload(std::string* payload)
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

void NgxplusIOBuf::print_payload(size_t capacity)
{
    std::string payload;
    payload.reserve(capacity);
    dump_payload(&payload);
    std::cout << payload << std::endl;
}

void NgxplusIOBuf::print_payload()
{
    print_payload(2048);
}

void NgxplusIOBuf::dump_info(std::string* info)
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
    common::string_appendf(info, "_blocks = %d\n", _blocks);
    common::string_appendf(info, "_read_block = %d\n", _read_block);
    common::string_appendf(info, "_read_point = %p\n", _read_point);
    common::string_appendf(info, "bytes = %zu\n", get_byte_count());
}

void NgxplusIOBuf::print_info(size_t capacity)
{
    std::string info;
    info.reserve(capacity);
    dump_info(&info);
    std::cout << info << std::endl;
}

void NgxplusIOBuf::print_info()
{
    print_info(2048);
}

}
