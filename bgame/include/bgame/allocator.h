#ifndef BGAME_ALLOCATOR_H
#define BGAME_ALLOCATOR_H

#include <stddef.h>
#include <autolist.h>
#include "reloadable.h"

typedef struct bgame_allocator_s {
	void* (*realloc)(void* ptr, size_t size, struct bgame_allocator_s* ctx);
} bgame_allocator_t;

static inline void*
bgame_realloc(void* ptr, size_t size, bgame_allocator_t* allocator) {
	return allocator->realloc(ptr, size, allocator);
}

static inline void*
bgame_malloc(size_t size, bgame_allocator_t* allocator) {
	return bgame_realloc(NULL, size, allocator);
}

static inline void
bgame_free(void* ptr, bgame_allocator_t* allocator) {
	bgame_realloc(ptr, 0, allocator);
}

extern bgame_allocator_t* bgame_default_allocator;

#define BGAME_DECLARE_TRACKED_ALLOCATOR(NAME) \
	AUTOLIST_ENTRY(bgame_tracked_allocator, bgame_allocator_t*, NAME) = NULL; \
	BGAME_PERSIST_VAR(NAME)

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
