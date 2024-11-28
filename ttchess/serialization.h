#ifndef TTCHESS_SERIALIZATION_H
#define TTCHESS_SERIALIZATION_H

#include <bgame/serialization.h>
#include <bgame/allocator.h>

static const bserial_ctx_config_t g_bserial_config = {
	.max_depth = 16,
	.max_num_symbols = 256,
	.max_record_fields = 32,
	.max_symbol_len = 64,
};

static inline bserial_ctx_t*
begin_serialize(bserial_in_t* in, bserial_out_t* out, bgame_allocator_t* allocator) {
	void* mem = bgame_malloc(bserial_ctx_mem_size(g_bserial_config), allocator);
	return bserial_make_ctx(mem, g_bserial_config, in, out);
}

static inline bserial_status_t
end_serialize(bserial_ctx_t* ctx, bgame_allocator_t* allocator) {
	bserial_status_t status = bserial_status(ctx);
	bgame_free(ctx, allocator);
	return status;
}

#endif
