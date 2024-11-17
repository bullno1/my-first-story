#include <bgame/scene.h>
#include <cute_app.h>

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

AUTOLIST_ENTRY(test_list, int, a) = 0;

BGAME_SCENE(main_scene) = {
	.update = update,
};
