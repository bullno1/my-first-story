#define REMODULE_HOST_IMPLEMENTATION
#include <bgame/reloadable.h>

#if defined(_WIN32)
#	define DYNLIB_EXT ".dll"
#elif defined(__APPLE__)
#	define DYNLIB_EXT ".dylib"
#elif defined(__linux__)
#	define DYNLIB_EXT ".so"
#endif

#define BGAME_LOADER_TARGET(APP_NAME) BGAME_LOADER_TARGET2(APP_NAME)
#define BGAME_LOADER_TARGET2(APP_NAME) #APP_NAME DYNLIB_EXT

extern int
bgame_loader_main(const char* name, int argc, const char** argv);

int main(int argc, const char* argv[]) {
	return bgame_loader_main(BGAME_LOADER_TARGET(BGAME_APP_NAME), argc, argv);
}
