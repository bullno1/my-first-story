#include <bgame/entrypoint.h>
#include <bgame/allocator.h>
#include <bgame/scene.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pico_log.h>
#include <cute_app.h>

BGAME_VAR(bool, app_created) = false;

static const char* WINDOW_TITLE = "bgame scratch";

extern bgame_scene_t* main_scene;

static void
init(int argc, const char** argv) {
	// Cute Framework
	if (!app_created) {
		log_info("Creating app");
		int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
		cf_make_app(WINDOW_TITLE, 0, 0, 0, 1280, 720, options, argv[0]);

		app_created = true;
	}

	cf_set_fixed_timestep(30);
	cf_app_set_vsync(true);
	cf_app_set_title(WINDOW_TITLE);

	if (bgame_current_scene() == NULL) {
		bgame_set_scene("main_scene");
	}
}

static void
report_allocator_stats(
	const char* name,
	bgame_tracked_allocator_t* allocator,
	void* userdata
) {
	log_debug("%s: Total %" PRId64 ", Peak %" PRId64, name, allocator->total, allocator->peak);
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
