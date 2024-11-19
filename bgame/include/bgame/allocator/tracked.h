#ifndef BGAME_TRACKED_ALLOCATOR_H

#include <bgame/reloadable.h>
#include <autolist.h>

#define BGAME_DECLARE_TRACKED_ALLOCATOR(NAME) \
	AUTOLIST_ENTRY(bgame_tracked_allocator_list, struct bgame_allocator_s*, NAME) = NULL; \
	BGAME_PERSIST_VAR(NAME)

struct bgame_allocator_s;

typedef struct bgame_allocator_stats_s {
	size_t total;
	size_t peak;
} bgame_allocator_stats_t;

void
bgame_enumerate_tracked_allocators(
	void (*fn)(const char* name, bgame_allocator_stats_t stats, void* userdata),
	void* userdata
);

#endif
