#include "internal.h"
#include <bgame/reloadable.h>
#include <bgame/log.h>
#include "loader_interface.h"

extern void
bgame_frame_allocator_next_frame(void);

#if BGAME_RELOADABLE

static void
bgame_update(bgame_loader_interface_t* interface) {
	bgame_frame_allocator_next_frame();
	interface->app.update();
}

void
bgame_remodule(bgame_app_t app, remodule_op_t op, void* userdata) {
	bgame_loader_interface_t* loader_interface = userdata;

	switch (op) {
		case REMODULE_OP_LOAD:
			bgame_init();
			loader_interface->app = app;
			loader_interface->update = bgame_update;
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
			bgame_init();

			log_info("App reloaded");
			loader_interface->app = app;
			loader_interface->update = bgame_update;
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
	bgame_init();

	app.init(argc, argv);
	while (cf_app_is_running()) {
		bgame_frame_allocator_next_frame();
		app.update();
	}
	app.cleanup();

	return 0;
}

#endif
