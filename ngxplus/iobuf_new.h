
#include "abstract_iobuf.h"

namespace ngxplus {

class IOBuf : public common::AbstractIOBuf
{
public:
	IOBuf(size_t block_size) : AbstractIOBuf(block_size) {}
	virtual ~IOBuf() {}

/* store and resume the read point at one time */
	void read_point_cache() {}
	bool read_point_resume() {return true;}

/* dump and print the infomation or payload */
    void dump_payload(std::string* payload) {(void) payload;}
    void print_payload(size_t capacity) {(void) capacity;}
    void dump_info(std::string* info) {(void) info;}
    void print_info(size_t capacity) {(void) capacity;}
	
protected:
	bool init_pool();

	bool alloc_next_block();
	bool is_current_block_buf_enough(size_t size, IOBufAllocType type);
	size_t current_block_remain_buf_size();
	size_t current_block_alloc_buf_size();
	char* alloc_from_current_block(size_t n);
	bool reclaim_to_current_block(size_t n);

	void move_read_point_to_next_block();
	size_t current_block_remain_data_size();
	size_t current_block_consume_data_size();

private:
    ngx_pool_t* _pool;
    ngx_pool_t* _read_pool;
    char* _start_points[IOBUF_MAX_BLOCKS_NUM];
};

}
