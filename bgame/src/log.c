#include "internal.h"
#define PICO_LOG_IMPLEMENTATION
#include <pico_log.h>

BGAME_INIT(bgame_logf) {
	log_appender_t id = log_add_stream(stderr, LOG_LEVEL_TRACE);
	log_set_time_fmt(id, "%H:%M:%S");
	log_display_colors(id, true);
	log_display_timestamp(id, true);
	log_display_file(id, true);
}
