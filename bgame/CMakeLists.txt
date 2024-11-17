set(SOURCES
	"src/entrypoint.c"
	"src/libs.c"
)

add_library(bgame STATIC "${SOURCES}")
target_compile_definitions(bgame PUBLIC BGAME_RELOADABLE=$<IF:$<BOOL:${RELOADABLE}>,1,0>)
target_include_directories(bgame PUBLIC include)
target_link_libraries(bgame PUBLIC cute $<$<BOOL:${RELOADABLE}>:remodule> pico_headers)
target_link_libraries(bgame PRIVATE cute $<$<BOOL:${RELOADABLE}>:blibs>)
