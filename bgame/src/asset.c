#include <bgame/reloadable.h>
#include <bgame/asset.h>
#include <bgame/allocator.h>
#include <bhash.h>

#if BGAME_RELOADABLE
#include <bresmon.h>
#endif

BGAME_DECLARE_TRACKED_ALLOCATOR(bgame_asset)

typedef struct {
	const char* type;
	const char* name;
} bgame_asset_key_t;

typedef struct {
	int dummy;
	_Alignas(max_align_t) char data[];
} bgame_asset_t;

typedef BHASH_TABLE(bgame_asset_key_t, bgame_asset_t*) bgame_asset_cache_t;

struct bgame_asset_bundle_s {
	bgame_asset_cache_t assets;
	// TODO: use the same allocator
	bresmon_t* resmon;
};

void
bgame_begin_load_assets(bgame_asset_bundle_t** bundle_ptr) {
}

bool
bgame_file_changed(bgame_asset_bundle_t* bundle, const char* file) {
	return false;
}

void*
bgame_load_asset(
	bgame_asset_bundle_t* bundle,
	const char* type,
	const char* path,
	const void* args
) {
	return NULL;
}

void
bgame_end_load_assets(bgame_asset_bundle_t* bundle) {
}

void*
bgame_asset_malloc(bgame_asset_bundle_t* bundle, size_t size) {
	return bgame_malloc(size, bgame_asset);
}

void
bgame_asset_free(bgame_asset_bundle_t* bundle, void* ptr) {
	bgame_free(ptr, bgame_asset);
}

void
bgame_check_assets(bgame_asset_bundle_t* bundle) {
}

void
bgame_unload_assets(bgame_asset_bundle_t* bundle) {
}
