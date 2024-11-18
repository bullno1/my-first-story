#include <bgame/allocator.h>
#include <cute_alloc.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pico_log.h>
#include <cute_app.h>

static void*
bgame_default_realloc(void* ptr, size_t size, bgame_allocator_t* ctx) {
	(void)ctx;
	if (size > 0) {
		return realloc(ptr, size);
	} else {
		free(ptr);
		return NULL;
	}
}

bgame_allocator_t g_bgame_default_allocator = {
	.realloc = bgame_default_realloc,
};

bgame_allocator_t* bgame_default_allocator = &g_bgame_default_allocator;

typedef struct {
	int_fast64_t size;
	_Alignas(max_align_t) char mem[];
} bgame_tracked_mem_t;

AUTOLIST_DECLARE(bgame_tracked_allocator)

typedef struct bgame_tracked_allocator_s {
	bgame_allocator_t allocator;

	atomic_int_fast64_t total;
	atomic_int_fast64_t peak;
} bgame_tracked_allocator_t;

void
bgame_enumerate_tracked_allocators(
	void (*fn)(const char* name, bgame_allocator_stats_t stats, void* userdata),
	void* userdata
) {
	AUTOLIST_FOREACH(itr, bgame_tracked_allocator) {
		bgame_tracked_allocator_t** allocator_ptr = (*itr)->value_addr;
		if (*allocator_ptr == NULL) { continue; }

		bgame_allocator_stats_t stats = {
			.peak = (size_t)atomic_load_explicit(&(*allocator_ptr)->peak, memory_order_relaxed),
			.total = (size_t)atomic_load_explicit(&(*allocator_ptr)->total, memory_order_relaxed),
		};
		fn((*itr)->name, stats, userdata);
	}
}

static void
bgame_tracked_allocator_adjust(bgame_tracked_allocator_t* allocator, int_fast64_t change) {
	int_fast64_t total = atomic_fetch_add(&allocator->total, change) + change;

	if (change > 0) {
		int_fast64_t peak = atomic_load(&allocator->peak);

		while (total > peak) {
			if (atomic_compare_exchange_weak(&allocator->peak, &peak, total)) {
				break;
			}
		}
	}
}

static void*
bgame_tracked_allocator_realloc(void* ptr, size_t size, bgame_allocator_t* ctx) {
	bgame_tracked_allocator_t* allocator = (bgame_tracked_allocator_t*)ctx;

	if (ptr == NULL) {
		if (size == 0) { return NULL; }

		bgame_tracked_mem_t* mem = bgame_realloc(NULL, size + sizeof(bgame_tracked_mem_t), bgame_default_allocator);
		mem->size = (int_fast64_t)size;
		bgame_tracked_allocator_adjust(allocator, (int_fast64_t)size);
		return mem->mem;
	} else {
		bgame_tracked_mem_t* mem = (void*)((char*)ptr - offsetof(bgame_tracked_mem_t, mem));
		int_fast64_t old_size = mem->size;

		if (size == 0) {
			bgame_realloc(mem, 0, bgame_default_allocator);
			bgame_tracked_allocator_adjust(allocator, -old_size);
			return NULL;
		} else {
			bgame_tracked_mem_t* new_mem = bgame_realloc(mem, size + sizeof(bgame_tracked_mem_t), bgame_default_allocator);
			new_mem->size = (int_fast64_t)size;
			bgame_tracked_allocator_adjust(allocator, (int_fast64_t)size - old_size);
			return new_mem->mem;
		}
	}
}

BGAME_DECLARE_TRACKED_ALLOCATOR(cute_framework)

static void*
bgame_cf_allocator_alloc(size_t size, void* udata) {
	(void)udata;
	return bgame_malloc(size, cute_framework);
}

static void
bgame_cf_allocator_free(void* ptr, void* udata) {
	(void)udata;
	bgame_free(ptr, cute_framework);
}

static void*
bgame_cf_allocator_calloc(size_t size, size_t count, void* udata) {
	(void)udata;
	void* mem = bgame_malloc(size * count, cute_framework);
	memset(mem, 0, size * count);
	return mem;
}

static void*
bgame_cf_allocator_realloc(void* ptr, size_t size, void* udata) {
	(void)udata;
	return bgame_realloc(ptr, size, cute_framework);
}

CF_Allocator bgame_cf_allocator = {
	.alloc_fn = bgame_cf_allocator_alloc,
	.free_fn = bgame_cf_allocator_free,
	.calloc_fn = bgame_cf_allocator_calloc,
	.realloc_fn = bgame_cf_allocator_realloc,
};

void
bgame_init_allocators(void) {
	cf_allocator_override(bgame_cf_allocator);

	AUTOLIST_FOREACH(itr, bgame_tracked_allocator) {
		bgame_tracked_allocator_t** allocator_ptr = (*itr)->value_addr;

		if (*allocator_ptr == NULL) {
			bgame_tracked_allocator_t* allocator = bgame_malloc(
				sizeof(bgame_tracked_allocator_t), bgame_default_allocator
			);

			*allocator = (bgame_tracked_allocator_t){
				.allocator.realloc = bgame_tracked_allocator_realloc,
			};

			*allocator_ptr = allocator;
		} else {
			(*allocator_ptr)->allocator.realloc = bgame_tracked_allocator_realloc;
		}
	}
}
