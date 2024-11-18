#ifndef BGAME_ASSET_H
#define BGAME_ASSET_H

#include <autolist.h>
#include <stddef.h>
#include <stdbool.h>

#define BGAME_ASSET_TYPE(NAME) \
	AUTOLIST_ENTRY(bgame_asset_type_list, bgame_asset_type_t, NAME)

typedef struct bgame_asset_bundle_s bgame_asset_bundle_t;

typedef enum {
	BGAME_ASSET_LOADED,
	BGAME_ASSET_UNCHANGED,
	BGAME_ASSET_ERROR,
} bgame_asset_load_result_t;

typedef bgame_asset_load_result_t (*bgame_asset_load_fn_t)(
	bgame_asset_bundle_t* bundle,
	void* asset,
	const char* path,
	const void* args
);
typedef void (*bgame_asset_unload_fn_t)(
	bgame_asset_bundle_t* bundle,
	void* asset
);

typedef struct bgame_asset_type_s {
	const char* name;

	size_t size;
	bgame_asset_load_fn_t load;
	bgame_asset_unload_fn_t unload;
} bgame_asset_type_t;

void
bgame_begin_load_assets(bgame_asset_bundle_t** bundle_ptr);

bool
bgame_file_changed(bgame_asset_bundle_t* bundle, const char* file);

void*
bgame_load_asset(
	bgame_asset_bundle_t* bundle,
	const char* type,
	const char* path,
	const void* args
);

void
bgame_end_load_assets(bgame_asset_bundle_t* bundle);

void*
bgame_asset_malloc(bgame_asset_bundle_t* bundle, size_t size);

void
bgame_asset_free(bgame_asset_bundle_t* bundle, void* ptr);

void
bgame_check_assets(bgame_asset_bundle_t* bundle);

void
bgame_unload_assets(bgame_asset_bundle_t* bundle);

#endif
