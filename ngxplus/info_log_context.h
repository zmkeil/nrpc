#ifndef NGXPLUS_INFO_LOG_CONTEXT
#define NGXPLUS_INFO_LOG_CONTEXT

#include <stdarg.h>
#include "log.h"
#include "timer.h"

/*
 * InfoLogContext is based Log, this is used to info Runtime Messages,
 * by default, this is flushed into logs/error.log
 */

#define LOG(_level_, _fmt_, args...)                                        \
    ngxplus::InfoLogContext::get_context()->log(_level_, "%s [%s:%d][%s] "  \
            _fmt_, ngxplus::asctime(), __FILE__, __LINE__,  \
            __FUNCTION__, ##args)                                           \

namespace ngxplus {

class InfoLogContext;
extern InfoLogContext* info_log_context;

class InfoLogContext
{
public:
    static InfoLogContext* get_context() {
        if (!info_log_context) {
            info_log_context = new InfoLogContext();
        }
        return info_log_context;
    }

    static void set_log_file(std::string& file_name) {
        INFO_LOG_NAME_DEFAULT = file_name;
    }

    static void set_log_level(int level) {
        INFO_LOG_LEVEL_DEFAULT = level;
    }

public:
    void log(int level, const char* fmt, ...) {
        va_list args;
        if (level <= INFO_LOG_LEVEL_DEFAULT) {
            va_start(args, fmt);
            _log.comlog_write_core(level, fmt, args);
            va_end(args);
        }
    }

private:
    InfoLogContext() {
        _log.init(INFO_LOG_NAME_DEFAULT, INFO_LOG_LEVEL_DEFAULT);
    }

private:
    Log _log;

private:
    static int INFO_LOG_LEVEL_DEFAULT;
    static std::string INFO_LOG_NAME_DEFAULT;
};

}
#endif
