#ifndef BGAME_LOADER_INTERFACE_H
#define BGAME_LOADER_INTERFACE_H

#include <bgame/app.h>

typedef struct bgame_loader_interface_s {
	int argc;
	const char** argv;

	bgame_app_t app;
	void (*update)(struct bgame_loader_interface_s* interface);
} bgame_loader_interface_t;

#endif
