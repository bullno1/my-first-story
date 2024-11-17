#include <bgame/reloadable.h>
#include <pico_log.h>
#include "loader_interface.h"

static void
bgame_init_logger(void) {
	log_appender_t id = log_add_stream(stderr, LOG_LEVEL_TRACE);
	log_set_time_fmt(id, "%H:%M:%S");
	log_display_colors(id, true);
	log_display_timestamp(id, true);
	log_display_file(id, true);
}

#if BGAME_RELOADABLE

void
bgame_remodule(bgame_app_t app, remodule_op_t op, void* userdata) {
	bgame_loader_interface_t* loader_interface = userdata;

	switch (op) {
		case REMODULE_OP_LOAD:
			bgame_init_logger();
			loader_interface->app = app;
			log_info("App loaded");
			break;
		case REMODULE_OP_UNLOAD:
			log_info("Unloading app");
			break;
		case REMODULE_OP_BEFORE_RELOAD:
			log_info("Reloading app");
			if (app.before_reload != NULL) {
				app.before_reload();
			}
			break;
		case REMODULE_OP_AFTER_RELOAD:
			bgame_init_logger();  // Logger contains callbacks
			log_info("App reloaded");
			loader_interface->app = app;
			if (app.after_reload != NULL) {
				app.after_reload();
			}
			log_info("Reinitializing");
			app.init(loader_interface->argc, loader_interface->argv);
			log_info("Reinitialized");
			break;
	}
}

#else

#include <cute_app.h>

int
bgame_static(bgame_app_t app, int argc, const char** argv) {
	bgame_init_logger();

	app.init(argc, argv);
	while (cf_app_is_running()) {
		app.update();
	}
	app.cleanup();
	return 0;
}

#endif
