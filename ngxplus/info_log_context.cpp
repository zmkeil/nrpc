#include "info_log_context.h"

namespace ngxplus {

std::string InfoLogContext::INFO_LOG_NAME_DEFAULT("error.log");
int InfoLogContext::INFO_LOG_LEVEL_DEFAULT = NGX_LOG_LEVEL_INFO;

// singlet
InfoLogContext* info_log_context = nullptr;

} // ngxplus
