#include <bgame/ui.h>
#include <bgame/ui/animation.h>
#include <clay.h>

void
bgame_ui_begin_animation(bgame_ui_animator_t* animator) {
	CLAY(
		CLAY_CUSTOM_ELEMENT({
			.type = BGAME_UI_CUSTOM_ANIMATION_BEGIN,
			.data.animation_begin.animator = animator
		}),
		CLAY_LAYOUT({
			.sizing = { 0 },
		})
	) {
	}
}

void
bgame_ui_end_animation(void) {
	CLAY(
		CLAY_CUSTOM_ELEMENT({ .type = BGAME_UI_CUSTOM_ANIMATION_END }),
		CLAY_LAYOUT({
			.sizing = { 0 },
		})
	) {
	}
}
