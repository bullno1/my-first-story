#ifndef BGAME_SCENE_H
#define BGAME_SCENE_H

typedef struct bgame_def_s bgame_scene_t;

void
bgame_set_scene(bgame_scene_t* scene);

void
bgame_current_scene(bgame_scene_t* scene);

void
bgame_scene_init(void);

void
bgame_scene_update(void);

void
bgame_scene_cleanup(void);

void
bgame_scene_before_reload(void);

void
bgame_scene_after_reload(void);

#endif
