#include <bgame/entrypoint.h>
#include <stdbool.h>
#include <pico_log.h>
#include <cute_graphics.h>
#include <cute_app.h>
#include <cute_draw.h>

BGAME_VAR(bool, app_created) = false;

static const char* WINDOW_TITLE = "bgame scratch";

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
}

static void
cleanup(void) {
	cf_destroy_app();
}

static void
fixed_update(void* udata) {
	(void)udata;
}

static void
update(void) {
	cf_app_update(fixed_update);

	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);

	cf_app_draw_onto_screen(true);
}

static bgame_app_t app = {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};

BGAME_ENTRYPOINT(app)
