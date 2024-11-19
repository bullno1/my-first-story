#ifndef BGAME_UI_ANIMATION_H
#define BGAME_UI_ANIMATION_H

#include <bgame/ui.h>

typedef struct bgame_ui_animator_s {
	void (*transition)(Clay_RenderCommand* from, const Clay_RenderCommand* to);
} bgame_ui_animator_t;

#endif
