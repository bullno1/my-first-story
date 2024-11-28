#include <bgame/entrypoint.h>
#include <bgame/scene.h>
#include <bgame/log.h>
#include <bgame/allocator/tracked.h>
#include <cute_app.h>
#include <cute_file_system.h>
#include <stdbool.h>

BGAME_VAR(bool, app_created) = false;

static const char* WINDOW_TITLE = "bgame scratch";

static void
init(int argc, const char** argv) {
	// Cute Framework
	if (!app_created) {
		int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
		cf_make_app(WINDOW_TITLE, 0, 0, 0, 1280, 720, options, argv[0]);

		// Mount assets dir
		char* base_dir = spnorm(cf_fs_get_base_directory());
		char* dir = sppopn(base_dir, 2);
		scat(dir, "/assets");
		CF_Result result = cf_fs_mount(dir, "/assets", true);
		if (result.code != CF_RESULT_SUCCESS) {
			log_warn("Could not mount %s: %s", dir, result.details);
		}
		sfree(dir);

		app_created = true;
	}

	cf_set_fixed_timestep(30);
	cf_app_set_vsync(true);
	cf_app_set_title(WINDOW_TITLE);

	if (bgame_current_scene() == NULL) {
		bgame_set_scene("scene_game");
	}
}

static void
report_allocator_stats(
	const char* name,
	bgame_allocator_stats_t stats,
	void* userdata
) {
	log_debug("%s: Total %" PRId64 ", Peak %" PRId64, name, stats.total, stats.peak);
}

static void
cleanup(void) {
	bgame_set_scene(NULL);
	cf_destroy_app();

	log_debug("--- Allocator stats ---");
	bgame_enumerate_tracked_allocators(report_allocator_stats, NULL);
}

static bgame_app_t app = {
	.init = init,
	.cleanup = cleanup,
	.update = bgame_scene_update,
	.before_reload = bgame_scene_before_reload,
	.after_reload = bgame_scene_after_reload,
};

BGAME_ENTRYPOINT(app)
