#ifndef BGAME_SCENE_H
#define BGAME_SCENE_H

#include <autolist.h>
#include "app.h"

typedef struct bgame_app_s bgame_scene_t;

#define BGAME_SCENE(NAME) \
	AUTOLIST_ENTRY(bgame_scene_list, bgame_scene_t, NAME)

void
bgame_set_scene(const char* name);

bgame_scene_t*
bgame_current_scene(void);

void
bgame_scene_update(void);

void
bgame_scene_before_reload(void);

void
bgame_scene_after_reload(void);

#endif
