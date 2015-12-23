#ifndef NGXPLUS_INFO_LOG_CONTEXT
#define NGXPLUS_INFO_LOG_CONTEXT

#include "log.h"

/*
 * InfoLogContext is based Log, this is used to info Runtime Messages,
 * by default, this is flushed into logs/error.log
 */

namespace ngxplus {

#define LOG(_level_, _fmt_, args...)                                        \
    ngxplus::InfoLogContext::get_context()->log(_level_, "[%s:%d][%s] "  \
            _fmt_, __FILE__, __LINE__, __FUNCTION__, ##args)            \

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
    const static int INFO_LOG_LEVEL_DEFAULT = NGX_LOG_LEVEL_INFO;
    const static std::string INFO_LOG_NAME_DEFAULT;
};

}
#endif
