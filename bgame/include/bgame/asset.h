#ifndef BGAME_ASSET_H
#define BGAME_ASSET_H

#include <stddef.h>

typedef void* (*basset_load_fn)(void* previous_instance, const char* path, const void* args);
typedef void (*basset_unload_fn)(void* asset);

typedef struct basset_def_s {
	const char* name;

	size_t arg_size;
	basset_load_fn load;
	basset_unload_fn unload;
} basset_def_t;

typedef struct basset_bundle_s basset_bundle_t;

void
basset_register(const basset_def_t* def);

void
basset_begin_bundle(basset_bundle_t** bundle_ptr);

void
basset_declare(
	basset_bundle_t* bundle,
	void** asset_ptr,
	const char* path,
	const void* args
);

void
basset_end_bundle(basset_bundle_t* bundle);

void
basset_load_bundle(basset_bundle_t* bundle);

void
basset_destroy_bundle(basset_bundle_t* bundle);

#endif
