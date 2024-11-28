#ifndef TTCHESS_TTCHESS_H
#define TTCHESS_TTCHESS_H

#include <stdbool.h>
#include <stdint.h>
#include <bserial.h>

#define TTCHESS_NUM_PLAYERS 2
#define TTCHESS_NUM_ERAS 3
#define TTCHESS_NUM_PAWNS 14
#define TTCHESS_NUM_STATUES 3
#define TTCHESS_BOARD_WIDTH 4
#define TTCHESS_BOARD_HEIGHT 4
#define TTCHESS_FIRST_WHITE_PAWN 0
#define TTCHESS_FIRST_BLACK_PAWN 7

typedef struct ttchess_config_s {
	bool with_statues;
} ttchess_config_t;

typedef struct ttchess_pos_s {
	int8_t x;
	int8_t y;
} ttchess_pos_t;

typedef enum {
	TTCHESS_COLOR_WHITE = 0,
	TTCHESS_COLOR_BLACK = 1,
} ttchess_color_t;

typedef enum {
	TTCHESS_ACTION_FIRST,
	TTCHESS_ACTION_SECOND,
	TTCHESS_ACTION_SHIFT_FOCUS,
	TTCHESS_ACTION_WON,
} ttchess_player_action_t;

typedef struct {
	ttchess_player_action_t action;
	ttchess_color_t color;
} ttchess_phase_t;

typedef enum {
	TTCHESS_PAWN_RESERVE = 0,
	TTCHESS_PAWN_PAST,
	TTCHESS_PAWN_PRESENT,
	TTCHESS_PAWN_FUTURE,
	TTCHESS_PAWN_DEAD,
} ttchess_pawn_status_t;

typedef enum {
	TTCHESS_ERA_PAST    = 0,
	TTCHESS_ERA_PRESENT = 1,
	TTCHESS_ERA_FUTURE  = 2,
} ttchess_era_t;

typedef enum {
	// Core
	TTCHESS_PIECE_NONE,
	TTCHESS_PIECE_PAWN,
	// Statue
	TTCHESS_PIECE_STATUE,
	// Plant
	// TTCHESS_PIECE_SHRUB,
	// TTCHESS_PIECE_STANDING_TREE,
	// TTCHESS_PIECE_FALLEN_TREE,
	// Elephant
	// TTCHESS_PIECE_ELEPHANT,
} ttchess_piece_type_t;

typedef struct ttchess_cell_s {
	ttchess_piece_type_t piece_type;
	int8_t piece_id;
} ttchess_cell_t;

typedef struct {
	int8_t num_pawns[TTCHESS_NUM_PLAYERS];
	ttchess_cell_t cells[TTCHESS_BOARD_WIDTH][TTCHESS_BOARD_HEIGHT];
} ttchess_board_t;

typedef struct {
	ttchess_pawn_status_t status;
	ttchess_pos_t pos;
} ttchess_pawn_t;

typedef struct {
	ttchess_pos_t positions[TTCHESS_NUM_ERAS];
} ttchess_statue_state_t;

typedef enum {
	// Core
	TTCHESS_MOVE_PAWN,
	TTCHESS_MOVE_TIME_TRAVEL,
	TTCHESS_MOVE_SHIFT_FOCUS,
	// Statue
	TTCHESS_MOVE_BUILD_STATUE,
	TTCHESS_MOVE_PULL_STATUE,
	// Plant
	// TTCHESS_MOVE_PLANT_SEED,
	// TTCHESS_MOVE_UNPLANT_SEED,
	// Elephant
	// TTCHESS_MOVE_TRAIN_ELEPHANT,
	// TTCHESS_MOVE_COMMAND_ELEPHANT,
} ttchess_move_type_t;

typedef struct {
	ttchess_move_type_t type;

	int8_t pawn_id;
	int8_t subject_id;
	union {
		ttchess_pos_t pos;
		ttchess_era_t era;
	};
} ttchess_move_t;

typedef struct ttchess_state_s {
	// Base states
	ttchess_config_t config;
	ttchess_phase_t phase;
	int8_t last_pawn;
	ttchess_pawn_t pawns[TTCHESS_NUM_PAWNS];
	ttchess_era_t focuses[TTCHESS_NUM_PLAYERS];
	ttchess_statue_state_t statues[TTCHESS_NUM_STATUES];

	// Derived states
	ttchess_board_t boards[TTCHESS_NUM_ERAS];
	bool statue_built[TTCHESS_NUM_STATUES];
} ttchess_state_t;

void
ttchess_init(ttchess_state_t* state, ttchess_config_t config);

int
ttchess_list_moves(
	const ttchess_state_t* state,
	ttchess_move_t* moves,
	int moves_len
);

bool
ttchess_apply_move(ttchess_state_t* state, ttchess_move_t move);

bserial_status_t
ttchess_serialize(bserial_ctx_t* ctx, ttchess_state_t* state);

#endif
