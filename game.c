v2 arena_half_size;

v2 player_p;
v2 player_dp;
v2 player_half_size;

b32 invicible;

v2 ball_p;
v2 ball_dp;
v2 ball_half_size;
b32 ball_base_speed;
f32 ball_speed_multiplier;
b32 first_ball_movement;

struct {
	v2 p;
	v2 half_size;
	f32 ball_speed_multiplier;
	int life;
	u32 color;
} typedef Block;

Block blocks[256];
int num_block;
int blocks_destroyed;

b32 initialized = false;

enum {
	GM_NORMAL,
	GM_WALL,
	GM_CONSTRUCTION,
	GM_SPACED,
	GM_POWERUP,
	/*GM_PONG,*/

	GM_COUNT,
} typedef Game_Mode;

enum {
	POWERUP_INACTIVE,
	POWERUP_INVICIBILITY,
	POWERUP_TRIPLESHOT,

	POWERUP_COUNT,
} typedef Powerup_Kind;

struct {
	Powerup_Kind kind;
	v2 p;
} typedef Powerup;

struct {
	Powerup powerups[16];
	int next_powerup;

	f32 invicibility_time;
	int number_of_triple_shots;

} typedef GM_Powerups_State;

struct {
	union {
		GM_Powerups_State;
	};
} typedef Game_Mode_State;

Game_Mode current_game_mode;
Game_Mode_State game_mode_state;

internal void
create_aligned_blocks(int num_x, int num_y, v2 block_half_size, f32 spacing_x, f32 spacing_y, f32 offset) {
	f32 x_offset = (f32)num_x * block_half_size.x * (2.f+spacing_x) *.5f - block_half_size.x*(1.f+spacing_x/2.f);
	f32 y_offset = offset;
	for (int y = 0; y < num_y; ++y) {
		for (int x = 0; x < num_x; ++x) {
			Block *block = blocks+num_block++;
			if (num_block >= array_count(blocks)) {
				num_block = 0;
			}
			block->life = 1;
			block->half_size = block_half_size;
			block->p.x = x*block->half_size.x*(2.f+spacing_x) - x_offset;
			block->p.y = y*block->half_size.y*(2.f+spacing_y) - y_offset;
			block->color = make_color_from_grey(y*255/num_y);
			block->ball_speed_multiplier = 1+ (f32)y*1.25f/(f32)num_y;
		}
	}
}

internal void
spawn_powerup(v2 p) {
	Powerup *powerup = game_mode_state.powerups + game_mode_state.next_powerup++;
	if (game_mode_state.next_powerup >= array_count(game_mode_state.powerups)) game_mode_state.next_powerup = 0;
	powerup->p = p;
	powerup->kind = POWERUP_INVICIBILITY;
}

internal void
restart_game(){

	if (current_game_mode >= GM_COUNT) current_game_mode = 0;
	else if (current_game_mode < 0) current_game_mode = GM_COUNT-1;
	game_mode_state = (Game_Mode_State){0};

	ball_base_speed = 50;
	ball_dp.x = 0;
	ball_dp.y = -ball_base_speed;
	ball_p.x = 0;
	ball_p.y = 40;
	ball_half_size = (v2){.75, .75};
	ball_speed_multiplier = 1.f;

	player_p.y = -35;
	player_half_size = (v2){10, 2};

	arena_half_size = (v2){85, 45};
	first_ball_movement = true;

	num_block = 0;
	blocks_destroyed = 0;
	for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
		block->life = 0;
	}

	switch(current_game_mode) {
		case GM_NORMAL: {
			create_aligned_blocks(20, 9, (v2){4, 2}, 0, 0, -4.f);
			
		} break;

		case GM_WALL: {
			int num_x = 15;
			int num_y = 9;
			v2 block_half_size = (v2){4, 2};
			f32 x_offset = ((f32)num_x * block_half_size.x * (2.5f)) * .5f - block_half_size.x*(1.f+.5f)*.5f;
			f32 y_offset = -1.f;
			for (int y = 0; y < num_y; ++y) {
				for (int x = 0; x < num_x; ++x) {
					Block *block = blocks+num_block++;
					if (num_block >= array_count(blocks)) {
						num_block = 0;
					}

					block->life = 1;
					block->half_size = block_half_size;

					if (y%2) block->p.x = x*block->half_size.x*2.5f - x_offset;
					else block->p.x = x*block->half_size.x*2.5f - x_offset + block->half_size.x;
					block->p.y = y*block->half_size.y*2.5f - y_offset;
					block->color = make_color_from_grey(y*255/num_y);
					block->ball_speed_multiplier = 1 + (f32)y*1.25f/(f32)num_y;
				}
			}
		} break;

		case GM_CONSTRUCTION: {
			create_aligned_blocks(21, 6, (v2){4, 2}, 0, 2.f, 0.f);
		} break;

		case GM_SPACED: {
			create_aligned_blocks(10, 6, (v2){4, 2}, 2.f, 2.f, 0.f);
		} break;

		invalid_default_case;
	}
}

internal void
test_for_win_condition() {
	if (blocks_destroyed == num_block) restart_game(++current_game_mode);
}

internal void
block_destroyed(Block *block) {
	++blocks_destroyed;

	switch(current_game_mode) {
		case GM_WALL: {
			spawn_powerup(block->p);
		} break;
	}

	test_for_win_condition();
}

internal void
simulate_game_mode() {
	switch(current_game_mode) {
		case GM_WALL: {
			for (Powerup *powerup = game_mode_state.powerups;
				 powerup != game_mode_state.powerups+array_count(game_mode_state.powerups);
				 ++powerup) {
				if (powerup->kind == POWERUP_INACTIVE) continue;
				draw_rect(powerup->p, (v2){2,2}, 0xffff00);
			}
		}
	}
}

#include "collision.c"

internal void
simulate_game(Input *input, f32 dt, b32 *running) {

	if (!initialized) {
		initialized = true;
		Game_Mode current_game_mode = 0;
		restart_game();
	}

	v2 player_desired_p;
	player_desired_p.x = pixels_to_world(input->mouse).x;
	player_desired_p.y = player_p.y;
	v2 ball_desired_p = add_v2(ball_p, mul_v2(ball_dp, dt));

	ball_colliding_player(&ball_desired_p);

	ball_colliding_arena(&ball_desired_p);

	clear_screen_and_draw_rect((v2){0, 0}, arena_half_size, 0xff0000, 0x551100);

	for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
		if (!block->life) continue;

		if (!first_ball_movement)
		// Ball/block collision
		{
			ball_colliding_block(ball_desired_p, block);
		}

		draw_rect(block->p, block->half_size, block->color);
	}

	ball_p = ball_desired_p;
	player_dp.x = (player_desired_p.x - player_p.x) / dt;
	player_p = player_desired_p;

	simulate_game_mode();

	draw_rect(ball_p, ball_half_size, 0x00ffff);

	if (invicible) draw_rect(player_p, player_half_size, 0xffffff);
	else draw_rect(player_p, player_half_size, 0x22ff22);

	// Check Win
	test_for_win_condition();

	// Game over
	if (ball_desired_p.y - ball_half_size.y < -50) {
		restart_game(current_game_mode);
	}

	if (pressed(BUTTON_ESC)) *running = false;

#if DEVELOPMENT
	if (pressed(BUTTON_LEFT)) restart_game(--current_game_mode);
	if (pressed(BUTTON_RIGHT)) restart_game(++current_game_mode);
#endif
}