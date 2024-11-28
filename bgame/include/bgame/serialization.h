#ifndef BGAME_SERIALIZATION
#define BGAME_SERIALIZATION

#include <cute_file_system.h>

#define BSERIAL_MEM
#include <bserial.h>

typedef struct bgame_cf_in_s {
	bserial_in_t bserial;
	CF_File* file;
} bserial_vfs_in_t;

typedef struct bgame_cf_out_s {
	bserial_out_t bserial;
	CF_File* file;
} bserial_vfs_out_t;

bserial_in_t*
bserial_vfs_begin_read(bserial_vfs_in_t* in, const char* name);

static inline void
bserial_vfs_end_read(bserial_vfs_in_t* in) {
	cf_fs_close(in->file);
}

bserial_out_t*
bserial_vfs_begin_write(bserial_vfs_out_t* out, const char* name);

void
bserial_vfs_end_write(bserial_vfs_out_t* out) {
	cf_fs_close(out->file);
}

#endif
