#include <bgame/scene.h>
#include <bgame/app.h>
#include <bgame/reloadable.h>
#include <bgame/log.h>
#include <string.h>
#include <stdlib.h>

static bgame_scene_t* g_bgame_current_scene = NULL;

char g_bgame_current_scene_name[128];
BGAME_PERSIST_VAR(g_bgame_current_scene_name)
BGAME_VAR(size_t, g_bgame_current_scene_name_len) = 0;

AUTOLIST_DECLARE(bgame_scene_list)

static inline bgame_scene_t*
bgame_find_scene(const char* name, size_t name_len) {
	AUTOLIST_FOREACH(itr, bgame_scene_list) {
		const autolist_entry_t* entry = *itr;

		if (
			entry->name_length == name_len
			&& strncmp(entry->name, name, entry->name_length) == 0
		) {
			return entry->value_addr;
		}
	}

	return NULL;
}

void
bgame_set_scene(const char* name) {
	bgame_scene_t* target_scene = NULL;
	size_t name_len = 0;

	if (name != NULL) {
		name_len = strlen(name);
		if (name_len > sizeof(g_bgame_current_scene_name)) {
			log_error("Scene name is too long: %s", name);
			return;
		}

		target_scene = bgame_find_scene(name, name_len);
		if (target_scene == NULL) {
			log_error("Could not find scene: %s", name);
			if (g_bgame_current_scene == NULL) { exit(1); }
			return;
		}
	}

	if (g_bgame_current_scene != NULL && g_bgame_current_scene->cleanup != NULL) {
		log_info("Cleaning up scene %s", g_bgame_current_scene_name);
		g_bgame_current_scene->cleanup();
	}

	g_bgame_current_scene = target_scene;
	if (name != NULL) {
		memcpy(g_bgame_current_scene_name, name, name_len);
	}
	g_bgame_current_scene_name_len = name_len;

	if (target_scene && target_scene->init != NULL) {
		log_info("Initializing scene %s", g_bgame_current_scene_name);
		target_scene->init(0, NULL);
	}
}

bgame_scene_t*
bgame_current_scene(void) {
	return g_bgame_current_scene;
}

void
bgame_scene_update(void) {
	if (g_bgame_current_scene != NULL && g_bgame_current_scene->update != NULL) {
		g_bgame_current_scene->update();
	}
}

void
bgame_scene_before_reload(void) {
	if (g_bgame_current_scene != NULL && g_bgame_current_scene->before_reload != NULL) {
		g_bgame_current_scene->before_reload();
	}
}

void
bgame_scene_after_reload(void) {
	g_bgame_current_scene = bgame_find_scene(
		g_bgame_current_scene_name, g_bgame_current_scene_name_len
	);

	if (g_bgame_current_scene == NULL) {
		log_error(
			"Could not restore scene: %.*s",
			(int)g_bgame_current_scene_name_len, g_bgame_current_scene_name
		);
		return;
	}

	if (g_bgame_current_scene->after_reload != NULL) {
		log_info("Reloading scene %s", g_bgame_current_scene_name);
		g_bgame_current_scene->after_reload();
	}

	if (g_bgame_current_scene->init != NULL) {
		log_info("Reinitializing scene %s", g_bgame_current_scene_name);
		g_bgame_current_scene->init(0, NULL);
	}
}
