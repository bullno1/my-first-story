#include <bgame/entrypoint.h>
#include <bgame/scene.h>
#include <bgame/log.h>
#include <bgame/allocator/tracked.h>
#include <cute_app.h>
#include <cute_file_system.h>
#include <stdbool.h>
#include "scenes/game.h"

BGAME_VAR(bool, app_created) = false;

static const char* WINDOW_TITLE = "ttchess";

static void
init(int argc, const char** argv) {
	// Cute Framework
	if (!app_created) {
		int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
		cf_make_app(WINDOW_TITLE, 0, 0, 0, 1280, 720, options, argv[0]);

		// Mount assets dir
		CF_Result result = cf_fs_mount("./assets", "/assets", true);
		if (result.code != CF_RESULT_SUCCESS) {
			log_warn("Could not mount %s: %s", ".", result.details);
		}

		cf_app_set_icon("/assets/icon.png");

		// Write dir
		cf_fs_set_write_directory(cf_fs_get_user_directory("bullno1", "ttchess"));

		app_created = true;
	}

	cf_set_fixed_timestep(30);
	cf_app_set_vsync(true);
	cf_app_set_title(WINDOW_TITLE);

	if (bgame_current_scene() == NULL) {
		goto_new_game_scene((ttchess_config_t){
			.with_statues = true,
		});
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
