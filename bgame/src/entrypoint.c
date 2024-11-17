#include <bgame/reloadable.h>
#include <bgame/app.h>
#include <cute_app.h>

#if BGAME_RELOADABLE

#include <bresmon.h>

typedef struct {
	int argc;
	const char** argv;
	bgame_app_t app;
} bgame_loader_interface_t;

static void
reload_module(const char* path, void* module) {
	printf("Reloading %s\n", path);
	remodule_reload(module);
}

void
bgame_remodule(bgame_app_t app, remodule_op_t op, void* userdata) {
	bgame_loader_interface_t* loader_interface = userdata;

	switch (op) {
		case REMODULE_OP_LOAD:
			loader_interface->app = app;
			break;
		case REMODULE_OP_UNLOAD:
			break;
		case REMODULE_OP_BEFORE_RELOAD:
			if (app.before_reload != NULL) {
				app.before_reload();
			}
			break;
		case REMODULE_OP_AFTER_RELOAD:
			loader_interface->app = app;
			if (app.after_reload != NULL) {
				app.after_reload();
			}
			app.init(loader_interface->argc, loader_interface->argv);
			break;
	}
}

int
bgame_loader_main(const char* name, int argc, const char** argv) {
	bgame_loader_interface_t loader_interface = {
		.argc = argc,
		.argv = argv,
	};

	remodule_t* module = remodule_load(name, &loader_interface);
	bresmon_t* monitor = bresmon_create(NULL);
	bresmon_watch(monitor, remodule_path(module), reload_module, module);

	loader_interface.app.init(argc, argv);

	while (cf_app_is_running()) {
		loader_interface.app.update();

		if (bresmon_should_reload(monitor, false)) {
			printf("Reloading\n");
			bresmon_reload(monitor);
			printf("Reloaded\n");
		}
	}

	loader_interface.app.cleanup();

	bresmon_destroy(monitor);
	remodule_unload(module);

	return 0;
}

#else

int
bgame_static(bgame_app_t app, int argc, const char** argv) {
	app.init(argc, argv);
	while (cf_app_is_running()) {
		app.update();
	}
	app.cleanup();
	return 0;
}

extern int bgame_entry(int argc, const char** argv);

int
bgame_loader_main(const char* name, int argc, const char** argv) {
	(void)name;
	return bgame_entry(argc, argv);
}

#endif
