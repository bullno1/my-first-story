#include <bgame/scene.h>
#include <bgame/allocator.h>
#include <bgame/allocator/tracked.h>
#include <bgame/log.h>
#include <bgame/asset.h>
#include <bgame/asset/sprite.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_sprite.h>
#include <cute_input.h>
#include "../serialization.h"
#include "bgame/serialization.h"
#include "game.h"

BGAME_DECLARE_TRACKED_ALLOCATOR(g_alloc_scene_game)
#define SCENE_ALLOCATOR g_alloc_scene_game

static const float CELL_SIZE = 75.f;
static const float BOARD_GAP = 50.f;
static const CF_Color CELL_WHITE = { 1.f, 1.f, 1.f, 1.f };
static const CF_Color CELL_BLACK = { 0.2f, 0.2f, 0.2f, 1.f };

static ttchess_state_t g_state;

BGAME_VAR(bserial_mem_out_t, g_saved_state) = { 0 };

BGAME_VAR(bgame_asset_bundle_t*, assets_game) = NULL;
BGAME_VAR(CF_Sprite*, spr_white_pawn) = { 0 };
BGAME_VAR(CF_Sprite*, spr_black_pawn) = { 0 };

static void
init(int argc, const char** argv) {
	bgame_asset_begin_load(&assets_game);

	spr_white_pawn = bgame_load_sprite(assets_game, "/assets/white-pawn.aseprite");
	spr_white_pawn->scale = (CF_V2) { 0.25f, 0.25f };
	cf_sprite_set_loop(spr_white_pawn, true);

	spr_black_pawn = bgame_load_sprite(assets_game, "/assets/black-pawn.aseprite");
	spr_black_pawn->scale = (CF_V2) { 0.25f, 0.25f };
	cf_sprite_set_loop(spr_black_pawn, true);

	bgame_asset_end_load(assets_game);

}

static void
cleanup(void) {
	bgame_free(g_saved_state.mem, SCENE_ALLOCATOR);
	g_saved_state.mem = NULL;
}

static void
serialize(bserial_in_t* in, bserial_out_t* out) {
	bserial_ctx_t* ctx = begin_serialize(in, out, SCENE_ALLOCATOR);
	if (ttchess_serialize(ctx, &g_state) != BSERIAL_OK) {
		log_error("Error during serialization");
	}
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
draw_board(const ttchess_state_t* state, ttchess_era_t era) {
	const float tilt_factor = 0.85f;
	const ttchess_board_t* board = &state->boards[era];
	for (int x = 0; x < TTCHESS_BOARD_WIDTH; ++x) {
		for (int y = 0; y < TTCHESS_BOARD_HEIGHT; ++y) {
			int color_selector = y + x;
			cf_draw_push_color(
				color_selector % 2 == 0
				? CELL_BLACK
				: CELL_WHITE
			);
			CF_V2 top_left = {
				 (float)x * CELL_SIZE,
				-(float)y * CELL_SIZE * tilt_factor,
			};
			CF_V2 bottom_left = {
				top_left.x,
				top_left.y - CELL_SIZE * tilt_factor,
			};
			CF_V2 bottom_right = {
				bottom_left.x + CELL_SIZE,
				bottom_left.y,
			};
			CF_V2 top_right = {
				bottom_right.x,
				bottom_right.y + CELL_SIZE * tilt_factor,
			};
			cf_draw_box_fill2(top_left, bottom_left, bottom_right, top_right, 0.f);
			cf_draw_pop_color();

			const ttchess_cell_t* cell = &board->cells[x][y];
			switch (cell->piece_type) {
				case TTCHESS_PIECE_NONE:
					break;
				case TTCHESS_PIECE_PAWN: {
					CF_V2 center = {
						top_left.x + CELL_SIZE * 0.5f,
						top_left.y - CELL_SIZE * 0.5f / tilt_factor,
					};
					ttchess_color_t color = ttchess_pawn_color(cell->piece_id);
					CF_Sprite* sprite = color == TTCHESS_COLOR_WHITE ? spr_white_pawn : spr_black_pawn;
					sprite->transform.p = center;
					cf_draw_sprite(sprite);
				} break;
				case TTCHESS_PIECE_STATUE:
					break;
			}
		}
	}
}

static void
fixed_update(void* arg) {
}

static void
update(void) {
	bgame_asset_check_bundle(assets_game);

	cf_sprite_update(spr_white_pawn);
	cf_sprite_update(spr_black_pawn);

	cf_app_update(fixed_update);
	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);

	float board_size = CELL_SIZE * TTCHESS_BOARD_WIDTH;
	float start_x = -board_size * 1.5f - BOARD_GAP;
	float start_y = board_size * 0.5f;
	for (int era = 0; era < TTCHESS_NUM_ERAS; ++era) {
		cf_draw_push();
		cf_draw_translate(
			start_x + (board_size + BOARD_GAP) * era,
			start_y
		);
		draw_board(&g_state, era);
		cf_draw_pop();
	}

	/*cf_draw_circle2((CF_V2){ 0.f, 0.f }, 10.f, 1.f);*/

	if (cf_mouse_just_pressed(CF_MOUSE_BUTTON_LEFT)) {
		printf("%f %f\n", cf_mouse_x(), cf_mouse_y());
	}

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
