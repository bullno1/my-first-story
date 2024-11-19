#ifndef BGAME_INTERNAL_H
#define BGAME_INTERNAL_H

#include <autolist.h>

typedef void (*bgame_on_init_fn_t)(void);

#define BGAME_INIT(NAME) \
	static void NAME##_init(void); \
	AUTOLIST_ENTRY(bgame_on_init_fns, bgame_on_init_fn_t, NAME) = NAME##_init; \
	static void NAME##_init(void)

void
bgame_init(void);

#ifdef _MSC_VER
#define BGAME_MAX_ALIGN_TYPE double
#else
#include <stddef.h>
#define BGAME_MAX_ALIGN_TYPE max_align_t
#endif

#endif
