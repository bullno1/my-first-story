#ifndef BGAME_FRAME_ALLOCATOR_H
#define BGAME_FRAME_ALLOCATOR_H

#include <stddef.h>

struct bgame_allocator_s;

extern struct bgame_allocator_s* bgame_frame_allocator;

void*
bgame_alloc_for_frame(size_t size, size_t alignment);

#endif
