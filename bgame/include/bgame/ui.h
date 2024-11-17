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

#endif
