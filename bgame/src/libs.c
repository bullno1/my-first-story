#include <bgame/reloadable.h>

#if BGAME_RELOADABLE

#define BLIB_IMPLEMENTATION
#include <bresmon.h>

#endif

#define PICO_LOG_IMPLEMENTATION
#include <pico_log.h>
BGAME_PERSIST_VAR(log_initialized)
BGAME_PERSIST_VAR(log_appender_count)
BGAME_PERSIST_VAR(log_appenders)
