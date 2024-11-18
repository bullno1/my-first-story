#include <bgame/reloadable.h>
#include <bgame/allocator.h>

#define PICO_LOG_IMPLEMENTATION
#include <pico_log.h>

#if BGAME_RELOADABLE

#define BRESMON_IMPLEMENTATION
#include <bresmon.h>

#endif

#define BLIB_REALLOC bgame_realloc
#define BHASH_IMPLEMENTATION
#include <bhash.h>
