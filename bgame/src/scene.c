#include <stddef.h>
#include <bgame/scene.h>
#include "reload.h"

BGAME_VAR(bgame_scene_t*, g_bgame_current_scene) = NULL;

void
bgame_set_scene(bgame_scene_t* scene) {
	if (g_bgame_current_scene != NULL && g_bgame_current_scene->cleanup != NULL) {
		g_bgame_current_scene->cleanup();
	}

	g_bgame_current_scene = scene;
	if (scene != NULL && scene->init != NULL) {
		scene->init();
	}
}

bgame_scene_t*
bgame_current_scene(void) {
	return g_bgame_current_scene;
}

void
bgame_scene_update(void) {
	if (bgame_current_scene != NULL && bgame_current_scene->update != NULL) {
		bgame_current_scene->update();
	}
}

void
bgame_scene_before_reload(void) {
	if (g_bgame_current_scene != NULL && g_bgame_current_scene->before_reload != NULL) {
		bgame_current_scene->before_reload();
	}
}

void
bgame_scene_after_reload(void) {
	if (g_bgame_current_scene != NULL && g_bgame_current_scene->after_reload != NULL) {
		bgame_current_scene->after_reload();
	}
}
