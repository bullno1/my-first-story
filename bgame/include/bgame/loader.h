#ifndef BGAME_LOADER_H
#define BGAME_LOADER_H

#define REMODULE_HOST_IMPLEMENTATION
#include "reloadable.h"

#define BGAME_LOADER(NAME) \
	int main(int argc, const char* argv[]) { \
		return bgame_loader_main(#NAME##REMODULE_DYNLIB_EXT, argc, argv); \
	}

int
bgame_loader_main(const char* name, int argc, const char** argv);

#endif
