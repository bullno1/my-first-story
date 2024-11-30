#include "ttchess.h"
#include <stdlib.h>

#define TTCHESS_NUM_CARDINAL_DIRECTIONS 4

typedef struct {
	ttchess_move_t* storage;
	int max_len;
	int len;
} ttchess_move_list_t;

static inline void
ttchess_add_move(ttchess_move_list_t* list, ttchess_move_t move) {
	int index = list->len++;
	if (index < list->max_len) {
		list->storage[index] = move;
	}
}

static inline bool
ttchess_pos_is_on_board(ttchess_pos_t pos) {
	return (0 <= pos.x && pos.x < TTCHESS_BOARD_WIDTH)
		&& (0 <= pos.y && pos.y < TTCHESS_BOARD_HEIGHT);
}

static inline ttchess_board_t*
ttchess_pawn_board(ttchess_state_t* state, int8_t pawn_id) {
	const ttchess_pawn_t* pawn = &state->pawns[pawn_id];
	switch (pawn->status) {
		case TTCHESS_PAWN_PAST:    return &state->boards[0];
		case TTCHESS_PAWN_PRESENT: return &state->boards[1];
		case TTCHESS_PAWN_FUTURE:  return &state->boards[2];
		default: return NULL;
	}
}

static inline void
ttchess_state_reindex(ttchess_state_t* state) {
	// Reset boards
	for (int era = 0; era < TTCHESS_NUM_ERAS; ++era) {
		ttchess_board_t* board = &state->boards[era];
		board->num_pawns[TTCHESS_COLOR_WHITE] = 0;
		board->num_pawns[TTCHESS_COLOR_BLACK] = 0;

		for (int x = 0; x < TTCHESS_BOARD_WIDTH; ++x) {
			for (int y = 0; y < TTCHESS_BOARD_HEIGHT; ++y) {
				board->cells[x][y].piece_type = TTCHESS_PIECE_NONE;
			}
		}
	}

	// Place pawns
	for (int pawn_index = 0; pawn_index < TTCHESS_NUM_PAWNS; ++pawn_index) {
		ttchess_pawn_t* pawn = &state->pawns[pawn_index];
		ttchess_board_t* pawn_board = ttchess_pawn_board(state, pawn_index);

		if (pawn_board != NULL) {
			pawn_board->cells[pawn->pos.x][pawn->pos.y].piece_type = TTCHESS_PIECE_PAWN;
			pawn_board->cells[pawn->pos.x][pawn->pos.y].piece_id = pawn_index;
			pawn_board->num_pawns[ttchess_pawn_color(pawn_index)] += 1;
		}
	}

	// Place statues
	if (state->config.with_statues) {
		for (int statue_index = 0; statue_index < TTCHESS_NUM_STATUES; ++statue_index) {
			state->statue_built[statue_index] = false;

			for (int era = 0; era < TTCHESS_NUM_ERAS; ++era) {
				ttchess_pos_t pos = state->statues[statue_index].positions[era];
				if (ttchess_pos_is_on_board(pos)) {
					state->statue_built[statue_index] = true;
					state->boards[era].cells[pos.x][pos.y].piece_type = TTCHESS_PIECE_STATUE;
					state->boards[era].cells[pos.x][pos.y].piece_id = statue_index;
				}
			}
		}
	}
}

void
ttchess_init(ttchess_state_t* state, ttchess_config_t config) {
	*state = (ttchess_state_t){
		.config = config,
		.phase = {
			.action = TTCHESS_ACTION_FIRST,
			.color = TTCHESS_COLOR_WHITE,
		},
		.focuses = {
			[TTCHESS_COLOR_WHITE] = TTCHESS_ERA_PAST,
			[TTCHESS_COLOR_BLACK] = TTCHESS_ERA_FUTURE,
		},
	};

	state->pawns[TTCHESS_FIRST_WHITE_PAWN + 0].status = TTCHESS_PAWN_PAST;
	state->pawns[TTCHESS_FIRST_WHITE_PAWN + 1].status = TTCHESS_PAWN_PRESENT;
	state->pawns[TTCHESS_FIRST_WHITE_PAWN + 2].status = TTCHESS_PAWN_FUTURE;
	for (int i = 0; i < TTCHESS_NUM_ERAS; ++i) {
		state->pawns[TTCHESS_FIRST_WHITE_PAWN + i].pos = (ttchess_pos_t){
			.x = 0,
			.y = 0,
		};
	}

	state->pawns[TTCHESS_FIRST_BLACK_PAWN + 0].status = TTCHESS_PAWN_PAST;
	state->pawns[TTCHESS_FIRST_BLACK_PAWN + 1].status = TTCHESS_PAWN_PRESENT;
	state->pawns[TTCHESS_FIRST_BLACK_PAWN + 2].status = TTCHESS_PAWN_FUTURE;
	for (int i = 0; i < TTCHESS_NUM_ERAS; ++i) {
		state->pawns[TTCHESS_FIRST_BLACK_PAWN + i].pos = (ttchess_pos_t){
			.x = 3,
			.y = 3,
		};
	}

	if (config.with_statues) {
		for (int era = 0; era < TTCHESS_NUM_ERAS; ++era) {
			for (int statue = 0; statue < TTCHESS_NUM_STATUES; ++statue) {
				state->statues[statue].positions[era] = (ttchess_pos_t){
					.x = -1,
					.y = -1,
				};
			}
		}

		for (int i = 0; i < TTCHESS_NUM_ERAS; ++i) {
			state->statues[TTCHESS_NUM_PLAYERS].positions[i] = (ttchess_pos_t){
				.x = 2,
				.y = 1,
			};
		}
	}

	ttchess_state_reindex(state);
}

static inline bool
ttchess_validate_move_pos_adjacent(const ttchess_pawn_t* pawn, ttchess_move_t move) {
	int delta_x = (int)move.pos.x - pawn->pos.x;
	int delta_y = (int)move.pos.y - pawn->pos.y;
	return (abs(delta_x) == 1 && delta_y == 0)
		|| (abs(delta_y) == 1 && delta_x == 0);
}

static inline bool
ttchess_piece_can_move(
	const ttchess_state_t* state,
	ttchess_era_t era,
	ttchess_pos_t from,
	ttchess_pos_t to,
	int depth
) {
	if (depth > TTCHESS_BOARD_WIDTH) { return false; }
	if (!ttchess_pos_is_on_board(from)) { return false; }
	ttchess_pos_t target_next = {
		.x = to.x + (to.x - from.x),
		.y = to.y + (to.y - from.y),
	};

	ttchess_cell_t piece = state->boards[era].cells[from.x][from.y];
	switch (piece.piece_type) {
		case TTCHESS_PIECE_NONE:
			// Cannot move nothing but can push things there
			return depth > 0;
		case TTCHESS_PIECE_PAWN: {
			// Pawn can be pushed off board but can't commit suicide
			if (!ttchess_pos_is_on_board(to)) {
				return depth > 0;
			}

			ttchess_cell_t target = state->boards[era].cells[to.x][to.y];
			switch (target.piece_type) {
				case TTCHESS_PIECE_NONE:
					return true;
				case TTCHESS_PIECE_PAWN:
					// Paradox, push or squish
					return true;
				case TTCHESS_PIECE_STATUE: {
					return ttchess_piece_can_move(state, era, to, target_next, depth + 1);
				}
			}
		}
		case TTCHESS_PIECE_STATUE: {
			// Statue cannot be pushed off board
			if (!ttchess_pos_is_on_board(to)) {
				return false;
			}
			ttchess_cell_t target = state->boards[era].cells[to.x][to.y];
			switch (target.piece_type) {
				case TTCHESS_PIECE_NONE:
					return true;
				case TTCHESS_PIECE_PAWN:
					// Paradox, push or squish
					return true;
				case TTCHESS_PIECE_STATUE: {
					return ttchess_piece_can_move(state, era, to, target_next, depth + 1);
				}
			}
		}
	}

	return false;
}

static inline bool
ttchess_validate_move(const ttchess_state_t* state, ttchess_move_t move) {
	ttchess_phase_t phase = state->phase;

	// Validate action type against phase
	switch (phase.action) {
		case TTCHESS_ACTION_FIRST:
		case TTCHESS_ACTION_SECOND:
			if (move.type == TTCHESS_MOVE_SHIFT_FOCUS) {
				return false;
			}
			break;
		case TTCHESS_ACTION_SHIFT_FOCUS:
			if (move.type != TTCHESS_MOVE_SHIFT_FOCUS) {
				return false;
			}
			break;
		case TTCHESS_ACTION_WON:
			return false;
	}

	// Because focus shift is done last, we can always validate pawn.
	// Most actions will involve a pawn anyway.

	// Range check
	int8_t pawn_id = phase.action == TTCHESS_ACTION_FIRST ? move.pawn_id : state->last_pawn;
	if (!(0 <= pawn_id && pawn_id < TTCHESS_NUM_PAWNS)) {
		return false;
	}

	// Player must own the pawn
	if (phase.color != ttchess_pawn_color(pawn_id)) { return false; }

	// Pawn must be on board
	const ttchess_pawn_t* pawn = &state->pawns[pawn_id];
	ttchess_era_t pawn_era = TTCHESS_ERA_PAST;
	switch (pawn->status) {
		case TTCHESS_PAWN_PAST:
			pawn_era = TTCHESS_ERA_PAST;
			break;
		case TTCHESS_PAWN_PRESENT:
			pawn_era = TTCHESS_ERA_PRESENT;
			break;
		case TTCHESS_PAWN_FUTURE:
			pawn_era = TTCHESS_ERA_FUTURE;
			break;
		default:
			return false;
	}

	// Pawn must be focused on the first action
	ttchess_era_t focused_era = state->focuses[phase.color];
	if (phase.action == TTCHESS_ACTION_FIRST && pawn_era != focused_era) {
		return false;
	}

	switch (move.type) {
		// Core
		case TTCHESS_MOVE_PAWN: {
			if (!ttchess_pos_is_on_board(move.pos)) { return false; }
			if (!ttchess_validate_move_pos_adjacent(pawn, move)) { return false; }

			return ttchess_piece_can_move(state, pawn_era, pawn->pos, move.pos, 0);
		}
		case TTCHESS_MOVE_TIME_TRAVEL: {
			// Valid era
			if (!(TTCHESS_ERA_PAST <= move.era && move.era <= TTCHESS_ERA_FUTURE)) {
				return false;
			}
			if (move.era == pawn_era) { return false; }

			// Target location must be empty
			ttchess_cell_t cell = state->boards[move.era].cells[pawn->pos.x][pawn->pos.y];
			if (cell.piece_type != TTCHESS_PIECE_NONE) { return false; }

			// To travel to an earlier era, there must be a spare pawn
			if (move.era < pawn_era) {
				int pawn_min = phase.color == TTCHESS_COLOR_WHITE
					? TTCHESS_FIRST_WHITE_PAWN
					: TTCHESS_FIRST_BLACK_PAWN;
				int pawn_max = pawn_min + TTCHESS_NUM_PAWNS / 2;
				for (int pawn_index = pawn_min; pawn_index < pawn_max; ++pawn_index) {
					if (state->pawns[pawn_index].status == TTCHESS_PAWN_RESERVE) {
						return true;
					}
				}
			}

			return false;
		}
		case TTCHESS_MOVE_SHIFT_FOCUS: {
			if (!(TTCHESS_ERA_PAST <= move.era && move.era <= TTCHESS_ERA_FUTURE)) {
				return false;
			}

			return move.era != focused_era;
		}
		// Statue
		case TTCHESS_MOVE_BUILD_STATUE: {
			if (!state->config.with_statues) { return false; }
			if (!ttchess_pos_is_on_board(move.pos)) { return false; }
			if (!ttchess_validate_move_pos_adjacent(pawn, move)) { return false; }
			if (state->statue_built[phase.color]) { return false; }

			ttchess_cell_t cell = state->boards[pawn_era].cells[move.pos.x][move.pos.y];
			return cell.piece_type == TTCHESS_PIECE_NONE;
		}
		case TTCHESS_MOVE_PULL_STATUE: {
			if (!ttchess_pos_is_on_board(move.pos)) { return false; }
			if (!ttchess_validate_move_pos_adjacent(pawn, move)) { return false; }

			ttchess_pos_t statue_pos = {
				.x = pawn->pos.x - (move.pos.x - pawn->pos.x),
				.y = pawn->pos.y - (move.pos.y - pawn->pos.y),
			};
			ttchess_cell_t cell = state->boards[pawn_era].cells[statue_pos.x][statue_pos.y];
			if (cell.piece_type != TTCHESS_PIECE_STATUE) {
				return false;
			}

			return ttchess_piece_can_move(state, pawn_era, pawn->pos, move.pos, 1);
		}
		// Plant
		// case TTCHESS_MOVE_PLANT_SEED: {
		// }
		// case TTCHESS_MOVE_UNPLANT_SEED: {
		// }
		// Elephant
		// case TTCHESS_MOVE_TRAIN_ELEPHANT: {
		// }
		// case TTCHESS_MOVE_COMMAND_ELEPHANT: {
		// }
	}

	return false;
}

static inline void
ttchess_add_move_if_legal(ttchess_move_list_t* move_list, const ttchess_state_t* state, ttchess_move_t move) {
	if (ttchess_validate_move(state, move)) {
		ttchess_add_move(move_list, move);
	}
}

static inline void
ttchess_gen_cardinal_positions(
	ttchess_pos_t positions[TTCHESS_NUM_CARDINAL_DIRECTIONS],
	ttchess_pos_t center
) {
	positions[0] = (ttchess_pos_t){ center.x - 1, center.y     };
	positions[1] = (ttchess_pos_t){ center.x + 1, center.y     };
	positions[2] = (ttchess_pos_t){ center.x    , center.y - 1 };
	positions[3] = (ttchess_pos_t){ center.x    , center.y + 1 };
}

static inline void
ttchess_add_pawn_moves(ttchess_move_list_t* move_list, const ttchess_state_t* state, int pawn_index) {
	// Time travel
	for (ttchess_era_t era = 0; era < TTCHESS_NUM_ERAS; ++era) {
		ttchess_add_move_if_legal(move_list, state, (ttchess_move_t){
			.type = TTCHESS_MOVE_TIME_TRAVEL,
			.pawn_id = pawn_index,
			.era = era,
		});
	}

	// Move pawn
	const ttchess_pawn_t* pawn = &state->pawns[pawn_index];
	ttchess_pos_t cardional_positions[TTCHESS_NUM_CARDINAL_DIRECTIONS];
	ttchess_gen_cardinal_positions(cardional_positions, pawn->pos);
	for (int i = 0; i < TTCHESS_NUM_CARDINAL_DIRECTIONS; ++i) {
		ttchess_add_move_if_legal(move_list, state, (ttchess_move_t){
			.type = TTCHESS_MOVE_PAWN,
			.pawn_id = pawn_index,
			.pos = cardional_positions[i],
		});
	}

	// Statue
	if (state->config.with_statues) {
		for (int i = 0; i < TTCHESS_NUM_CARDINAL_DIRECTIONS; ++i) {
			ttchess_add_move_if_legal(move_list, state, (ttchess_move_t){
				.type = TTCHESS_MOVE_BUILD_STATUE,
				.pawn_id = pawn_index,
				.pos = cardional_positions[i],
			});

			for (int statue_index = 0; statue_index < TTCHESS_NUM_STATUES; ++statue_index) {
				ttchess_add_move_if_legal(move_list, state, (ttchess_move_t){
					.type = TTCHESS_MOVE_PULL_STATUE,
					.pawn_id = pawn_index,
					.subject_id = statue_index,
					.pos = cardional_positions[i],
				});
			}
		}
	}
}

int
ttchess_list_moves(
	const ttchess_state_t* state,
	ttchess_move_t* moves,
	int moves_len
) {
	ttchess_move_list_t move_list = {
		.storage = moves,
		.max_len = moves_len,
	};
	switch (state->phase.action) {
		case TTCHESS_ACTION_FIRST: {
			ttchess_era_t focused_era = state->focuses[state->phase.color];
			int pawn_min = state->phase.color == TTCHESS_COLOR_WHITE
				? TTCHESS_FIRST_WHITE_PAWN
				: TTCHESS_FIRST_BLACK_PAWN;
			int pawn_max = pawn_min + TTCHESS_NUM_PAWNS / 2;
			for (int pawn_index = pawn_min; pawn_index < pawn_max; ++pawn_index) {
				const ttchess_pawn_t* pawn = &state->pawns[pawn_index];
				if (
					   (pawn->status == TTCHESS_PAWN_PAST    && focused_era == TTCHESS_ERA_PAST)
					|| (pawn->status == TTCHESS_PAWN_PRESENT && focused_era == TTCHESS_ERA_PRESENT)
					|| (pawn->status == TTCHESS_PAWN_FUTURE  && focused_era == TTCHESS_ERA_FUTURE)
				) {
					ttchess_add_pawn_moves(&move_list, state, pawn_index);
				}
			}
		}
		case TTCHESS_ACTION_SECOND: {
			ttchess_add_pawn_moves(&move_list, state, state->last_pawn);
		}
		case TTCHESS_ACTION_SHIFT_FOCUS: {
			ttchess_era_t focused_era = state->focuses[state->phase.color];
			for (ttchess_era_t era = 0; era < TTCHESS_NUM_ERAS; ++era) {
				// Must shift focus to a different era
				if (focused_era != era) {
					ttchess_add_move(&move_list, (ttchess_move_t){
						.type = TTCHESS_MOVE_SHIFT_FOCUS,
						.era = era,
					});
				}
			}
		}
		case TTCHESS_ACTION_WON:
			break;
	}

	return move_list.len;
}

static inline void
ttchess_kill_pawn(ttchess_state_t* state, int8_t pawn_id) {
	ttchess_pawn_t* pawn = &state->pawns[pawn_id];
	ttchess_color_t pawn_color = ttchess_pawn_color(pawn_id);
	ttchess_era_t pawn_era = TTCHESS_ERA_PAST;
	switch (pawn->status) {
		case TTCHESS_PAWN_PAST:
			pawn_era = TTCHESS_ERA_PAST;
			break;
		case TTCHESS_PAWN_PRESENT:
			pawn_era = TTCHESS_ERA_PRESENT;
			break;
		case TTCHESS_PAWN_FUTURE:
			pawn_era = TTCHESS_ERA_FUTURE;
			break;
		default:
			break;
	}
	state->boards[pawn_era].cells[pawn->pos.x][pawn->pos.y].piece_id = TTCHESS_PIECE_NONE;
	state->boards[pawn_era].num_pawns[pawn_color] -= 1;
	pawn->status = TTCHESS_PAWN_DEAD;
}

static inline void
ttchess_push_piece(
	ttchess_state_t* state,
	ttchess_era_t era,
	ttchess_pos_t from,
	ttchess_pos_t to
) {
	ttchess_board_t* board = &state->boards[era];
	ttchess_cell_t* cell = &board->cells[from.x][from.y];
	int8_t piece_id = cell->piece_id;
	int delta_x = to.x - from.x;
	int delta_y = to.y - from.y;
	ttchess_pos_t push_target = {
		.x = to.x + delta_x,
		.y = to.y + delta_y,
	};
	switch (cell->piece_type) {
		case TTCHESS_PIECE_NONE:
			break;
		case TTCHESS_PIECE_PAWN: {
			ttchess_pawn_t* pawn = &state->pawns[piece_id];
			if (ttchess_pos_is_on_board(to)) {
				ttchess_cell_t* target_cell = &board->cells[to.x][to.y];
				bool can_move = false;
				switch (target_cell->piece_type) {
					case TTCHESS_PIECE_NONE: {
						can_move = true;
					} break;
					case TTCHESS_PIECE_PAWN: {
						ttchess_color_t own_color = ttchess_pawn_color(piece_id);
						ttchess_color_t target_pawn_color = ttchess_pawn_color(target_cell->piece_id);
						if (own_color == target_pawn_color) {  // Paradox
							ttchess_kill_pawn(state, piece_id);
							ttchess_kill_pawn(state, target_cell->piece_id);
							can_move = false;
						} else {  // Push opponent
							ttchess_push_piece(state, era, to, push_target);
							can_move = true;
						}
					} break;
					default: {
						// Try to push target away
						if (ttchess_piece_can_move(state, era, to, push_target, 1)) {
							ttchess_push_piece(state, era, to, push_target);
							can_move = true;
						} else {
							ttchess_kill_pawn(state, piece_id);
							can_move = false;
						}
					} break;
				}

				// Actually move pawn
				if (can_move) {
					pawn->pos = to;
					cell->piece_type = TTCHESS_PIECE_NONE;
					target_cell->piece_type = TTCHESS_PIECE_PAWN;
					target_cell->piece_id = piece_id;
				}
			} else {  // Pushed off board
				// This is just so that the render can draw effects where the
				// pawn falls off the board
				pawn->pos.x += delta_x;
				pawn->pos.y += delta_y;
				ttchess_kill_pawn(state, piece_id);
			}
		} break;
		case TTCHESS_PIECE_STATUE: {
			ttchess_statue_state_t* statue = &state->statues[piece_id];
			// Propagate move through time
			for (int statue_era = era; era < TTCHESS_NUM_ERAS; ++era) {
				ttchess_pos_t statue_pos = statue->positions[era];
				if (!ttchess_pos_is_on_board(statue_pos)) { break; }

				// Try to move statue relatively
				ttchess_pos_t statue_to = {
					.x = statue_pos.x + delta_x,
					.y = statue_pos.y + delta_y,
				};
				ttchess_pos_t statue_push_target = {
					.x = statue_to.x + delta_x,
					.y = statue_to.y + delta_y,
				};
				// Clear target
				if (ttchess_piece_can_move(state, statue_era, statue_to, statue_push_target, 1)) {
					ttchess_push_piece(state, era, statue_to, statue_push_target);
				} else {
					break;
				}

				// Move statue
				statue->positions[era] = statue_to;
				state->boards[era].cells[statue_pos.x][statue_pos.y].piece_type = TTCHESS_PIECE_NONE;
				state->boards[era].cells[statue_to.x][statue_to.y].piece_type = TTCHESS_PIECE_STATUE;
				state->boards[era].cells[statue_to.x][statue_to.y].piece_id = piece_id;
			}
		} break;
	}
}

bool
ttchess_apply_move(ttchess_state_t* state, ttchess_move_t move) {
	if (!ttchess_validate_move(state, move)) { return false; }

	int8_t pawn_id = state->last_pawn;
	if (state->phase.action == TTCHESS_ACTION_FIRST) {
		state->last_pawn = pawn_id = move.pawn_id;
	}
	ttchess_pawn_t* pawn = &state->pawns[pawn_id];
	ttchess_era_t pawn_era = TTCHESS_ERA_PAST;
	ttchess_pos_t pawn_pos = pawn->pos;
	ttchess_color_t player_color = state->phase.color;

	switch (pawn->status) {
		case TTCHESS_PAWN_PAST:
			pawn_era = TTCHESS_ERA_PAST;
			break;
		case TTCHESS_PAWN_PRESENT:
			pawn_era = TTCHESS_ERA_PRESENT;
			break;
		case TTCHESS_PAWN_FUTURE:
			pawn_era = TTCHESS_ERA_FUTURE;
			break;
		default:
			return false;
	}

	switch (move.type) {
		// Core
		case TTCHESS_MOVE_PAWN:
			ttchess_push_piece(state, pawn_era, pawn->pos, move.pos);
			break;
		case TTCHESS_MOVE_TIME_TRAVEL: {
			// Change board
			ttchess_pawn_status_t pawn_status = pawn->status;
			switch (move.era) {
				case TTCHESS_ERA_PAST:
					pawn->status = TTCHESS_PAWN_PAST;
					break;
				case TTCHESS_ERA_PRESENT:
					pawn->status = TTCHESS_PAWN_PRESENT;
					break;
				case TTCHESS_ERA_FUTURE:
					pawn->status = TTCHESS_PAWN_FUTURE;
					break;
			}

			// Update board
			ttchess_board_t* current_board = &state->boards[pawn_era];
			ttchess_board_t* next_board = &state->boards[move.era];
			current_board->num_pawns[player_color] -= 1;
			current_board->cells[pawn_pos.x][pawn_pos.y].piece_type = TTCHESS_PIECE_NONE;
			next_board->num_pawns[player_color] += 1;
			next_board->cells[pawn_pos.x][pawn_pos.y].piece_type = TTCHESS_PIECE_PAWN;
			next_board->cells[pawn_pos.x][pawn_pos.y].piece_id = pawn_id;

			// Leave clone behind
			if (move.era < pawn_era) {
				int pawn_min = player_color == TTCHESS_COLOR_WHITE
					? TTCHESS_FIRST_WHITE_PAWN
					: TTCHESS_FIRST_BLACK_PAWN;
				int pawn_max = pawn_min + TTCHESS_NUM_PAWNS / 2;
				for (
					int new_pawn_index = pawn_min;
					new_pawn_index < pawn_max;
					++new_pawn_index
				) {
					ttchess_pawn_t* new_pawn = &state->pawns[new_pawn_index];
					if (new_pawn->status == TTCHESS_PAWN_RESERVE) {
						new_pawn->pos = pawn_pos;
						new_pawn->status = pawn_status;
						current_board->num_pawns[player_color] += 1;
						current_board->cells[pawn_pos.x][pawn_pos.y].piece_type = TTCHESS_PIECE_PAWN;
						current_board->cells[pawn_pos.x][pawn_pos.y].piece_id = new_pawn_index;
						break;
					}
				}
			}
		} break;
		case TTCHESS_MOVE_SHIFT_FOCUS:
			state->focuses[player_color] = move.era;
			break;
		// Statue
		case TTCHESS_MOVE_BUILD_STATUE: {
			// Build the current era
			state->statues[player_color].positions[pawn_era] = move.pos;
			ttchess_board_t* current_board = &state->boards[pawn_era];
			current_board->cells[move.pos.x][move.pos.y].piece_type = TTCHESS_PIECE_STATUE;
			current_board->cells[move.pos.x][move.pos.y].piece_id = player_color;

			// Propagate to later eras
			for (int era = pawn_era + 1; era < TTCHESS_NUM_ERAS; ++era) {
				ttchess_board_t* board = &state->boards[era];
				ttchess_cell_t* cell = &board->cells[move.pos.x][move.pos.y];
				ttchess_pos_t push_target = {
					.x = move.pos.x + (move.pos.x - pawn->pos.x),
					.y = move.pos.y + (move.pos.y - pawn->pos.y),
				};
				// Push whatever in the way away if possible for the build
				if (ttchess_piece_can_move(state, era, move.pos, push_target, 1)) {
					ttchess_push_piece(state, era, move.pos, push_target);

					cell->piece_type = TTCHESS_PIECE_STATUE;
					cell->piece_id = player_color;
					state->statues[player_color].positions[era] = move.pos;
				} else {  // If not, halt propagation
					break;
				}
			}
			state->statue_built[player_color] = true;
		} break;
		case TTCHESS_MOVE_PULL_STATUE: {
			ttchess_push_piece(state, pawn_era, pawn->pos, move.pos);
			ttchess_pos_t statue_pos = {
				.x = pawn->pos.x - (move.pos.x - pawn->pos.x),
				.y = pawn->pos.y - (move.pos.y - pawn->pos.y),
			};
			ttchess_push_piece(state, pawn_era, statue_pos, pawn->pos);
		} break;
		// Plant
		// TTCHESS_MOVE_PLANT_SEED,
		// TTCHESS_MOVE_UNPLANT_SEED,
		// Elephant
		// TTCHESS_MOVE_TRAIN_ELEPHANT,
		// TTCHESS_MOVE_COMMAND_ELEPHANT,
	}

	// Advance phase
	switch (state->phase.action) {
		case TTCHESS_ACTION_FIRST:
			state->phase.action = TTCHESS_ACTION_SECOND;
			break;
		case TTCHESS_ACTION_SECOND:
			state->phase.action = TTCHESS_ACTION_SHIFT_FOCUS;
			break;
		case TTCHESS_ACTION_SHIFT_FOCUS: {
			ttchess_color_t opposing_color = player_color == TTCHESS_COLOR_WHITE
				? TTCHESS_COLOR_BLACK
				: TTCHESS_COLOR_WHITE;
			// Victory check
			int num_dominant_eras = 0;
			for (int era = TTCHESS_ERA_PAST; era <= TTCHESS_ERA_FUTURE; ++era) {
				if (state->boards[era].num_pawns[opposing_color] == 0) {
					++num_dominant_eras;
				}
			}
			if (num_dominant_eras >= 2) {
				state->phase.action = TTCHESS_ACTION_WON;
				return true;
			}

			// Pass turn
			state->phase.action = TTCHESS_ACTION_FIRST;
			state->phase.color = opposing_color;
		} break;
		case TTCHESS_ACTION_WON:
			return false;
	}

	// When there is no legal moves for the next player, skip their turn
	if (ttchess_list_moves(state, NULL, 0) == 0) {
		state->phase.action = TTCHESS_ACTION_FIRST;
		// Flip color from latest state because the active player can do
		// something really dumb and cost them their second action.
		state->phase.color = state->phase.color == TTCHESS_COLOR_WHITE
			? TTCHESS_COLOR_BLACK
			: TTCHESS_COLOR_WHITE;
	}

	return true;
}

static inline bserial_status_t
ttchess_fixed_array(bserial_ctx_t* ctx, int len) {
	uint64_t u64_len = (uint64_t)len;
	BSERIAL_CHECK_STATUS(bserial_array(ctx, &u64_len));
	if (u64_len != (uint64_t)len) { return BSERIAL_MALFORMED; }

	return BSERIAL_OK;
}

static inline bserial_status_t
ttchess_fixed_table(bserial_ctx_t* ctx, int len) {
	uint64_t u64_len = (uint64_t)len;
	BSERIAL_CHECK_STATUS(bserial_table(ctx, &u64_len));
	if (u64_len != (uint64_t)len) { return BSERIAL_MALFORMED; }

	return BSERIAL_OK;
}

static inline bserial_status_t
ttchess_serialize_pos(bserial_ctx_t* ctx, ttchess_pos_t* pos) {
	BSERIAL_RECORD(ctx, pos) {
		BSERIAL_KEY(ctx, x) {
			BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &pos->x));
		}
		BSERIAL_KEY(ctx, y) {
			BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &pos->y));
		}
	}

	return BSERIAL_OK;
}

bserial_status_t
ttchess_serialize(bserial_ctx_t* ctx, ttchess_state_t* state) {
	BSERIAL_RECORD(ctx, state) {
		BSERIAL_KEY(ctx, config) {
			// `state->config` has the same address as `state` and that confuses
			// bserial
			ttchess_config_t config = state->config;
			BSERIAL_RECORD(ctx, &config) {
				BSERIAL_KEY(ctx, with_statues) {
					BSERIAL_CHECK_STATUS(bserial_bool(ctx, &config.with_statues));
				}
			}
			state->config = config;
		}

		BSERIAL_KEY(ctx, phase) {
			BSERIAL_RECORD(ctx, &state->phase) {
				BSERIAL_KEY(ctx, action) {
					uint8_t action = (uint8_t)state->phase.action;
					BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &action));
					state->phase.action = (ttchess_player_action_t)action;
				}

				BSERIAL_KEY(ctx, color) {
					uint8_t color = (uint8_t)state->phase.color;
					BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &color));
					state->phase.color = (ttchess_color_t)color;
				}
			}
		}

		BSERIAL_KEY(ctx, last_pawn) {
			BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &state->last_pawn));
			if (!(0 <= state->last_pawn && state->last_pawn < TTCHESS_NUM_PAWNS)) {
				return BSERIAL_MALFORMED;
			}
		}

		BSERIAL_KEY(ctx, pawns) {
			BSERIAL_CHECK_STATUS(ttchess_fixed_table(ctx, TTCHESS_NUM_PAWNS));
			for (int pawn_index = 0; pawn_index < TTCHESS_NUM_PAWNS; ++pawn_index) {
				ttchess_pawn_t* pawn = &state->pawns[pawn_index];

				BSERIAL_RECORD(ctx, pawn) {
					BSERIAL_KEY(ctx, status) {
						uint8_t status = (uint8_t)pawn->status;
						BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &status));
						pawn->status = (ttchess_pawn_status_t)status;
					}

					BSERIAL_KEY(ctx, pos) {
						BSERIAL_CHECK_STATUS(ttchess_serialize_pos(ctx, &pawn->pos));
					}
				}
			}
		}

		BSERIAL_KEY(ctx, focuses) {
			BSERIAL_CHECK_STATUS(ttchess_fixed_array(ctx, TTCHESS_NUM_PLAYERS));
			for (int i = 0; i < TTCHESS_NUM_PLAYERS; ++i) {
				uint8_t era = (uint8_t)state->focuses[i];
				BSERIAL_CHECK_STATUS(bserial_any_int(ctx, &era));
				state->focuses[i] = (ttchess_era_t)era;
			}
		}

		BSERIAL_KEY(ctx, statues) {
			BSERIAL_CHECK_STATUS(ttchess_fixed_table(ctx, TTCHESS_NUM_STATUES));
			for (int statue_index = 0; statue_index < TTCHESS_NUM_STATUES; ++statue_index) {
				ttchess_statue_state_t* statue = &state->statues[statue_index];

				BSERIAL_RECORD(ctx, statue) {
					BSERIAL_KEY(ctx, positions) {
						BSERIAL_CHECK_STATUS(ttchess_fixed_table(ctx, TTCHESS_NUM_ERAS));
						for (int era = 0; era < TTCHESS_NUM_ERAS; ++era) {
							BSERIAL_CHECK_STATUS(ttchess_serialize_pos(ctx, &statue->positions[era]));
						}
					}
				}
			}
		}
	}

	if (bserial_mode(ctx) == BSERIAL_MODE_READ) {
		ttchess_state_reindex(state);
	}

	return BSERIAL_OK;
}
