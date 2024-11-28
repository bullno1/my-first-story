#include <cute_app.h>
#include <bgame/scene.h>
#include <bgame/allocator.h>
#include <bgame/allocator/tracked.h>
#include <bgame/log.h>
#include "../serialization.h"
#include "game.h"

BGAME_DECLARE_TRACKED_ALLOCATOR(g_alloc_scene_game)
#define SCENE_ALLOCATOR g_alloc_scene_game

static ttchess_state_t g_state;

BGAME_VAR(bserial_mem_out_t, g_saved_state) = { 0 };

static void
init(int argc, const char** argv) {
}

static void
cleanup(void) {
	bgame_free(g_saved_state.mem, SCENE_ALLOCATOR);
	g_saved_state.mem = NULL;
}

static void
serialize(bserial_in_t* in, bserial_out_t* out) {
	bserial_ctx_t* ctx = begin_serialize(in, out, SCENE_ALLOCATOR);
	ttchess_serialize(ctx, &g_state);
	bserial_status_t status = end_serialize(ctx, SCENE_ALLOCATOR);

	if (status != BSERIAL_OK) {
		log_error("Error during serialization");
	}
}

static void
before_reload(void) {
	bserial_out_t* out = bserial_mem_init_out(&g_saved_state, SCENE_ALLOCATOR);
	serialize(NULL, out);
}

static void
after_reload(void) {
	bserial_mem_in_t mem_in;
	bserial_in_t* in = bserial_mem_init_in(&mem_in, g_saved_state.mem, g_saved_state.len);
	serialize(in, NULL);

	bgame_free(g_saved_state.mem, SCENE_ALLOCATOR);
	g_saved_state.mem = NULL;
}

static void
update(void) {
	cf_app_update(NULL);
	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);

	cf_app_draw_onto_screen(true);
}

void
goto_new_game_scene(ttchess_config_t config) {
	ttchess_init(&g_state, config);
	bgame_set_scene("game");
}

void
goto_saved_game_scene(ttchess_state_t state) {
	g_state = state;
	bgame_set_scene("game");
}

BGAME_SCENE(game) = {
	.init = init,
	.cleanup = cleanup,
	.update = update,
	.before_reload = before_reload,
	.after_reload = after_reload,
};
