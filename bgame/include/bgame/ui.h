#ifndef BGAME_UI_H
#define BGAME_UI_H

struct cute_9_patch_s;
#define CLAY_EXTEND_CONFIG_TEXT const char* fontName;
#define CLAY_EXTEND_CONFIG_RECTANGLE struct cute_9_patch_s* nine_patch;
#define CLAY_ const char* fontName;
#include <clay.h>
#include <cute_sprite.h>
#include <cute_color.h>

void
bgame_ui_begin(void);

void
bgame_ui_end(void);

static inline Clay_Color
bgame_ui_color(CF_Color color) {
	return (Clay_Color) {
		.r = color.r * 255.f,
		.g = color.g * 255.f,
		.b = color.b * 255.f,
		.a = color.a * 255.f,
	};
}

#endif
