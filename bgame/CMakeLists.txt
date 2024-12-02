set(SOURCES
	"src/libs.c"
	"src/log.c"
	"src/allocator.c"
	"src/allocator/tracked.c"
	"src/allocator/frame.c"
	"src/allocator/cute_framework.c"
	"src/entrypoint.c"
	"src/scene.c"
	"src/serialization.c"
	"src/ui.c"
	"src/asset.c"
	"src/asset/9patch.c"
	"src/asset/sprite.c"
	"src/internal.c"
)

add_library(bgame STATIC "${SOURCES}")
target_compile_definitions(bgame PUBLIC BGAME_RELOADABLE=$<IF:$<BOOL:${RELOADABLE}>,1,0>)
target_include_directories(bgame PUBLIC include)
target_link_libraries(bgame PUBLIC
	cute
	$<$<BOOL:${RELOADABLE}>:remodule>
	clay
	blibs
	pico_headers
)

add_library(bgame-loader STATIC "src/loader.c")
target_compile_definitions(bgame-loader PUBLIC BGAME_RELOADABLE=$<IF:$<BOOL:${RELOADABLE}>,1,0>)
target_include_directories(bgame-loader PRIVATE include)
target_link_libraries(bgame-loader PUBLIC cute $<$<BOOL:${RELOADABLE}>:remodule> blibs)

add_library(bgame-loader-stub STATIC "src/loader_stub.c")
