#include "internal.h"

AUTOLIST_DECLARE(bgame_on_init_fns)

void
bgame_init(void) {
	AUTOLIST_FOREACH(itr, bgame_on_init_fns) {
		const autolist_entry_t* entry = *itr;
		bgame_on_init_fn_t init_fn = *(bgame_on_init_fn_t*)entry->value_addr;
		init_fn();
	}
}
