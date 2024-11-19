#ifndef BGAME_ALLOCATOR_H
#define BGAME_ALLOCATOR_H

#include <bgame/reloadable.h>
#include <stddef.h>

typedef struct bgame_allocator_s {
	void* (*realloc)(void* ptr, size_t size, struct bgame_allocator_s* ctx);
} bgame_allocator_t;

extern bgame_allocator_t* bgame_default_allocator;

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

#endif
