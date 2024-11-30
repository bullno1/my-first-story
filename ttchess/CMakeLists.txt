add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gen/glow_shd.h
	COMMAND cute-shaderc
		-I${CMAKE_CURRENT_SOURCE_DIR}/../assets
		-type=draw
		-varname=s_glow_shd_bytecode
		-oheader=${CMAKE_CURRENT_SOURCE_DIR}/gen/glow_shd.h
		${CMAKE_CURRENT_SOURCE_DIR}/../assets/glow.shd
	DEPENDS ../assets/glow.shd
	DEPENDS cute-shaderc
)

set(SOURCES
	"main.c"
	"ttchess.c"
	"gen/glow_shd.h"
	"scenes/game.c"
)
add_bgame_app(ttchess "${SOURCES}")
