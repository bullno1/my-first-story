#include <bgame/reloadable.h>
#include <bgame/allocator.h>

#define BLIB_REALLOC bgame_realloc
#define BLIB_IMPLEMENTATION
#include <bhash.h>
#include <barray.h>
#include <barena.h>
#include <bgame/serialization.h>

#if BGAME_RELOADABLE
#include <bresmon.h>
#endif
