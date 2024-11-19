#ifndef BGAME_UI_ANIMATION_H
#define BGAME_UI_ANIMATION_H

#include <bgame/ui.h>

typedef struct bgame_ui_animator_s {
	void (*transition)(Clay_RenderCommand* from, const Clay_RenderCommand* to);
} bgame_ui_animator_t;

void
bgame_ui_begin_animation(bgame_ui_animator_t* animator);

void
bgame_ui_end_animation(void);

#define BGAME_UI_ANIMATION(...) \
	for ( \
		int BGAME__UI_DEFER_VAR = (bgame_ui_begin_animation(__VA_ARGS__), 0); \
		BGAME__UI_DEFER_VAR < 1; \
		++BGAME__UI_DEFER_VAR, bgame_ui_end_animation() \
	)

#endif
