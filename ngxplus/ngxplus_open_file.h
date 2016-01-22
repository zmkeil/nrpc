#ifndef NGXPLUS_FILE
#define NGXPLUS_FILE

extern "C" {
//#include <ngx_config.h>
#include <ngx_core.h>
}
#include "common_head.h"

namespace ngxplus {

class OpenFile
{
public:
    OpenFile() : _fd(-1) {}

    bool init(const std::string& full_name, int mode, int is_create, int access);

    bool open(int mode, int is_create, int access);

    bool close();

    int read(u_char* str, size_t len);

    int write(u_char* str, size_t len);

    bool ftell(long* offset);

    bool fseek(long offset, int whence);

    std::string get_name() {
        return _name;
    }

private:
    const char* mode2char(int mode);

private:
    std::string _name;
    ngx_fd_t _fd;
    FILE* _stream;
};

} // ngxplus

#endif
