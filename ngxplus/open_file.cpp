#include "info_log_context.h"
#include "open_file.h"

namespace ngxplus {

bool OpenFile::init(const std::string& full_name, int mode, int is_create, int access)
{
    _name = full_name;
    return open(mode, is_create, access);
}

bool OpenFile::open(int mode, int is_create, int access)
{
    _fd = ::ngx_open_file(_name.c_str(), mode/*NGX_FILE_APPEND*/,
            is_create/*NGX_FILE_CREATE_OR_OPEN|O_BINARY*/, access/*NGX_FILE_DEFAULT_ACCESS*/);
    if (_fd == NGX_INVALID_FILE) {
        LOG(NGX_LOG_LEVEL_ALERT, "could not open error log file: \"%s\"",
                _name.c_str());
        return false;
    }

    _stream = fdopen(_fd, mode2char(mode));
    if (!_stream) {
        LOG(NGX_LOG_LEVEL_ALERT, "open file \"%s\" stream failed",
                _name.c_str());
    }
    return true;
}

bool OpenFile::close()
{
    if (_fd == -1) {
        return true;
    }
    if (::ngx_close_file(_fd) == NGX_FILE_ERROR) {
        LOG(NGX_LOG_LEVEL_ALERT, "close error log file: \"%s\" failed",
                _name.c_str());
        return false;
    }
    _fd = -1;
    _stream = nullptr;
    return true;
}

int OpenFile::read(u_char* str, size_t len)
{
    int n;

    if (_fd < 0) {
        LOG(NGX_LOG_LEVEL_ALERT, "could not read file: \"%s\" before open",
                _name.c_str());
        return -1;
    }

    n = ::ngx_read_fd(_fd, str, len);
    if (n == -1) {
        LOG(NGX_LOG_LEVEL_ALERT, "read file: \"%s\" failed",
                _name.c_str());
        close();
    } else if ((size_t)n < len) {
        LOG(NGX_LOG_LEVEL_ALERT, "has only read %u of %u from \"%s\"",
                n, len, _name.c_str());
    }
    return n;
}
   
int OpenFile::write(u_char* str, size_t len)
{
    int n;

    if (_fd < 0) {
        LOG(NGX_LOG_LEVEL_ALERT, "could not write file: \"%s\" before open",
                _name.c_str());
        return -1;
    }

    n = ::ngx_write_fd(_fd, str, len);
    if (n == -1) {
        LOG(NGX_LOG_LEVEL_ALERT, "write file: \"%s\" failed",
                _name.c_str());
        close();
    } else if ((size_t)n < len) {
        LOG(NGX_LOG_LEVEL_ALERT, "has only write %u of %u to \"%s\"",
                n, len, _name.c_str());
    }
    return n;
}

bool OpenFile::ftell(long* offset)
{
    long ret = ::ftell(_stream);
    if (ret == -1) {
        LOG(NGX_LOG_LEVEL_ALERT, "ftell failed \"%s\"",
                _name.c_str());
        return false;
    }

    *offset = ret;
    return true;
}

bool OpenFile::fseek(long offset, int whence)
{
    if ((whence != SEEK_SET)
      & (whence != SEEK_CUR)
      & (whence != SEEK_END)) {
        LOG(NGX_LOG_LEVEL_ALERT, "fseek flags not supported \"%s\"",
                _name.c_str());
        return false;
    }

    if (::fseek(_stream, offset, whence) == -1) {
        LOG(NGX_LOG_LEVEL_ALERT, "fseek \"%s\" failed",
                _name.c_str());
        return false;
    }
    return true;
}

const char* OpenFile::mode2char(int mode)
{
    if (mode & O_APPEND) {
        return "a+";
    }
    if (mode & O_RDWR) {
        return "w+";
    }
    if (mode & O_WRONLY) {
        return "w";
    }
    if (mode & O_RDONLY) {
        return "r";
    }
    return "r+";
}

}
