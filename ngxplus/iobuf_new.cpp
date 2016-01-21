
#include "iobuf.h"

namespace ngxplus {

bool IOBuf::init_pool()
{
	_pool = ngx_create_pool(_block_size + 2 * sizeof(ngx_pool_t), NULL);
	if (!_pool) {
		LOG(ALERT, "create pool [blocksize=%ld] failed", _block_size);
		return -1;
	}
	if (_pool->max < _block_size) {
		LOG(ALERT, "pool's payload_size [%ld] LT pool.max [%ld]", _block_size, _pool->max);
		return -1;
	}
	_read_pool = _pool;
}

bool alloc_next_block();
{
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
}

	bool is_current_block_buf_enough(size_t size, IOBufAllocType type);
	size_t current_block_remain_buf_size();
	size_t current_block_alloc_buf_size();
	char* alloc_from_current_block(size_t n);
	bool reclaim_to_current_block(size_t n);

	void move_read_point_to_next_block();
	size_t current_block_remain_data_size();
	size_t current_block_consume_data_size();

}
