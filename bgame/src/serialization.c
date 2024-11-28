#include <bgame/serialization.h>
#include <cute_file_system.h>

static size_t
bserial_vfs_read(bserial_in_t* in, void* buf, size_t size) {
	return cf_fs_read(((bserial_vfs_in_t*)in)->file, buf, size);
}

static bool
bserial_vfs_skip(bserial_in_t* in, size_t size) {
	CF_File* file = ((bserial_vfs_in_t*)in)->file;
	return cf_fs_seek(file, cf_fs_tell(file) + size).code == CF_RESULT_SUCCESS;
}

static size_t
bserial_vfs_write(bserial_out_t* out, const void* buf, size_t size) {
	return cf_fs_write(((bserial_vfs_out_t*)out)->file, buf, size);
}

bserial_in_t*
bserial_vfs_begin_read(bserial_vfs_in_t* in, const char* name) {
	CF_File* file = cf_fs_open_file_for_read(name);
	if (file == NULL) { return NULL; }

	*in = (bserial_vfs_in_t){
		.bserial = {
			.read = bserial_vfs_read,
			.skip = bserial_vfs_skip,
		},
		.file = file,
	};

	return &in->bserial;
}

bserial_out_t*
bserial_vfs_begin_write(bserial_vfs_out_t* out, const char* name) {
	CF_File* file = cf_fs_open_file_for_write(name);
	if (file == NULL) { return NULL; }

	*out = (bserial_vfs_out_t){
		.bserial = {
			.write = bserial_vfs_write,
		},
		.file = file,
	};

	return &out->bserial;
}
