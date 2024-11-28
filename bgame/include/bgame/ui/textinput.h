#ifndef BGAME_UI_TEXTINPUT_H
#define BGAME_UI_TEXTINPUT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct bgame_ui_textinput_options_s {
	bool multiline;
	uint32_t max_len;
	uint32_t* len;
	char* chars;
} bgame_ui_textinput_options_t;

typedef enum {
	BGAME_UI_TEXTINPUT_NO_EVENT,
	BGAME_UI_TEXTINPUT_CHANGED,
	BGAME_UI_TEXTINPUT_FOCUS_GAINED,
	BGAME_UI_TEXTINPUT_FOCUS_LOST,
	BGAME_UI_TEXTINPUT_SUBMITTED,
} bgame_ui_textinput_event_t;

typedef struct bgame_ui_textinput_result_s {
	bgame_ui_textinput_event_t event;
} bgame_ui_textinput_result_t;

bgame_ui_textinput_result_t
bgame_ui_textinput(bgame_ui_textinput_options_t options);

#endif
