
#include "io/abstract_iobuf.h"

struct ngx_pool_s;
typedef struct ngx_pool_s ngx_pool_t;

namespace ngxplus {

class NgxplusIOBuf : public common::AbstractIOBuf
{
public:
	NgxplusIOBuf(size_t block_size) : AbstractIOBuf(block_size) {
        _pool = nullptr;
        _read_pool = nullptr;
        memset(_start_points, 0, sizeof(char*) * IOBUF_MAX_BLOCKS_NUM);
    }
	virtual ~NgxplusIOBuf() {}

/* store and resume the read point at one time */
	void read_point_cache() {}
	bool read_point_resume() {return true;}

/* dump and print the infomation or payload */
    void dump_payload(std::string* payload);// {(void) payload;}
    void print_payload(size_t capacity);// {(void) capacity;}
    void print_payload();// {print_payload(1024);}
    void dump_info(std::string* info);// {(void) info;}
    void print_info(size_t capacity);// {(void) capacity;}
    void print_info();// {print_info(1024);}
	
protected:
	bool init_pool();

	bool alloc_next_block();
	bool is_current_block_buf_enough(size_t size, common::IOBufAllocType type);
	size_t current_block_remain_buf_size();
	size_t current_block_alloc_buf_size();
	char* alloc_from_current_block(size_t n);
	bool reclaim_to_current_block(size_t n);

	void move_read_point_to_next_block();
	size_t current_block_remain_data_size();
	size_t current_block_consume_data_size();

private:
    static const int IOBUF_MAX_BLOCKS_NUM = 10;

private:
    ngx_pool_t* _pool;
    ngx_pool_t* _read_pool;
    char* _start_points[IOBUF_MAX_BLOCKS_NUM];
};

}
