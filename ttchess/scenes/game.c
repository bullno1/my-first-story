#include <bgame/scene.h>
#include <bgame/allocator.h>
#include <bgame/allocator/tracked.h>
#include <bgame/log.h>
#include <bgame/asset.h>
#include <bgame/asset/sprite.h>
#include <bgame/utils.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_graphics.h>
#include <cute_sprite.h>
#include <cute_input.h>
#include <cute_math.h>
#include "../gen/glow_shd.h"
#include "../serialization.h"
#include "bgame/serialization.h"
#include "game.h"

BGAME_DECLARE_TRACKED_ALLOCATOR(g_alloc_scene_game)
#define SCENE_ALLOCATOR g_alloc_scene_game

static const float CELL_SIZE = 75.f;
static const float BOARD_GAP = 80.f;
static const float BOARD_Y_SCALE = 0.85f;
static const CF_Color CELL_WHITE = { 1.f, 1.f, 1.f, 1.f };
static const CF_Color CELL_BLACK = { 0.3f, 0.3f, 0.3f, 1.f };
static const CF_Color SELECTION_GLOW = { 1.0f, 1.0f, 0.0f, 1.f };
static const float GLOW_THICKNESS = 3.f;

typedef enum {
	RENDER_LAYER_GLOW_PREPARE = 1,
	RENDER_LAYER_BOARD,
	RENDER_LAYER_GLOW,
	RENDER_LAYER_PIECES,
	RENDER_LAYER_UI,
} render_layer_t;

static ttchess_state_t g_state;

BGAME_VAR(bserial_mem_out_t, g_saved_state) = { 0 };

BGAME_VAR(bgame_asset_bundle_t*, assets_game) = NULL;
BGAME_VAR(CF_Sprite*, spr_white_pawn) = { 0 };
BGAME_VAR(CF_Sprite*, spr_black_pawn) = { 0 };
BGAME_VAR(CF_Canvas, canvas_glow) = { 0 };
BGAME_VAR(CF_Shader, shd_glow) = { 0 };

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

	if (shd_glow.id != 0) {
		cf_destroy_shader(shd_glow);
	}
	if (canvas_glow.id == 0) {
		int w, h;
		cf_app_get_size(&w, &h);
		CF_CanvasParams params = cf_canvas_defaults(w, h);
		params.target.wrap_u = CF_WRAP_MODE_CLAMP_TO_EDGE;
		params.target.wrap_v = CF_WRAP_MODE_CLAMP_TO_EDGE;
		canvas_glow = cf_make_canvas(params);
	}

	shd_glow = cf_make_draw_shader_from_bytecode(s_glow_shd_bytecode);
}

static void
cleanup(void) {
	bgame_free(g_saved_state.mem, SCENE_ALLOCATOR);
	g_saved_state.mem = NULL;

	bgame_asset_destroy_bundle(assets_game);
	cf_destroy_shader(shd_glow);
	cf_destroy_canvas(canvas_glow);
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
	const ttchess_board_t* board = &state->boards[era];
	CF_V2 mouse_pos = cf_screen_to_world((CF_V2){
		.x = (float)cf_mouse_x(),
		.y = (float)cf_mouse_y(),
	});
	for (int x = 0; x < TTCHESS_BOARD_WIDTH; ++x) {
		for (int y = 0; y < TTCHESS_BOARD_HEIGHT; ++y) {
			CF_V2 top_left = {
				 (float)x * CELL_SIZE,
				-(float)y * CELL_SIZE * BOARD_Y_SCALE,
			};
			CF_V2 bottom_left = {
				top_left.x,
				top_left.y - CELL_SIZE * BOARD_Y_SCALE,
			};
			CF_V2 top_right = {
				top_left.x + CELL_SIZE,
				top_left.y
			};
			CF_Aabb aabb = {
				.min = bottom_left,
				.max = top_right,
			};
			bool hovered = cf_contains_point(aabb, mouse_pos);

			BGAME_SCOPE(
				cf_draw_push_layer(RENDER_LAYER_BOARD),
				cf_draw_pop_layer()
			) {
				int color_selector = y + x;
				BGAME_SCOPE(
					cf_draw_push_color(
						color_selector % 2 == 0
						? CELL_BLACK
						: CELL_WHITE
					),
					cf_draw_pop_color()
				) {
					cf_draw_box_fill(aabb, 1.f);
				}
			}

			const ttchess_cell_t* cell = &board->cells[x][y];
			switch (cell->piece_type) {
				case TTCHESS_PIECE_NONE:
					break;
				case TTCHESS_PIECE_PAWN: {
					CF_V2 center = {
						top_left.x + CELL_SIZE * 0.5f,
						top_left.y - CELL_SIZE * 0.5f / BOARD_Y_SCALE,
					};
					ttchess_color_t color = ttchess_pawn_color(cell->piece_id);
					CF_Sprite* sprite = color == TTCHESS_COLOR_WHITE ? spr_white_pawn : spr_black_pawn;
					sprite->transform.p = center;

					render_layer_t layer = hovered ? RENDER_LAYER_GLOW_PREPARE : RENDER_LAYER_PIECES;
					BGAME_SCOPE(cf_draw_push_layer(layer), cf_draw_pop_layer()) {
						cf_draw_sprite(sprite);
					}
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

	BGAME_SCOPE(
		cf_draw_push_layer(RENDER_LAYER_UI),
		cf_draw_pop_layer()
	) {
		ttchess_era_t white_focus = g_state.focuses[TTCHESS_COLOR_WHITE];
		BGAME_SCOPE(
			cf_draw_push_color(CELL_WHITE),
			cf_draw_pop_color()
		) {
			cf_draw_arrow(
				(CF_V2){
					start_x + (board_size + BOARD_GAP) * (float)white_focus + board_size * 0.5f,
					start_y + 60.f,
				},
				(CF_V2){
					start_x + (board_size + BOARD_GAP) * (float)white_focus + board_size * 0.5f,
					start_y + 20.f,
				},
				10.f,
				20.f
			);
		}

		ttchess_era_t black_era = g_state.focuses[TTCHESS_COLOR_BLACK];
		BGAME_SCOPE(
			cf_draw_push_color(CELL_WHITE),
			cf_draw_pop_color()
		) {
			cf_draw_arrow(
				(CF_V2){
					start_x + (board_size + BOARD_GAP) * (float)black_era + board_size * 0.5f,
					start_y - board_size * BOARD_Y_SCALE - 60.f,
				},
				(CF_V2){
					start_x + (board_size + BOARD_GAP) * (float)black_era + board_size * 0.5f,
					start_y - board_size * BOARD_Y_SCALE - 20.f,
				},
				10.f,
				20.f
			);
		}
	}

	// Render canvas for glow
	cf_clear_color(SELECTION_GLOW.r, SELECTION_GLOW.g, SELECTION_GLOW.b, 0.f);
	cf_render_layers_to(canvas_glow, RENDER_LAYER_GLOW_PREPARE, RENDER_LAYER_GLOW_PREPARE, true);

	// Render things before glow
	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);
	cf_render_layers_to(cf_app_get_canvas(), RENDER_LAYER_GLOW_PREPARE + 1, RENDER_LAYER_GLOW - 1, true);

	// Render glow
	{
		int w, h;
		cf_app_get_size(&w, &h);

		cf_draw_push_shader(shd_glow);
		cf_draw_set_uniform_float("thickness", GLOW_THICKNESS);
		cf_draw_push_antialias(false);
		cf_draw_canvas(canvas_glow, (CF_V2){ 0, 0 }, (CF_V2){ w, h });
		cf_draw_pop_antialias();
		cf_draw_pop_shader();
	}

	cf_app_draw_onto_screen(false);
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
