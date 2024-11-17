#ifndef BGAME_LOADER_INTERFACE_H
#define BGAME_LOADER_INTERFACE_H

#include <bgame/app.h>

typedef struct {
	int argc;
	const char** argv;
	bgame_app_t app;
} bgame_loader_interface_t;

#endif
