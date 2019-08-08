#include "collectibles.c"
#include "game_struct.c"

v2 arena_half_size;

v2 player_p;
v2 player_desired_p;
v2 player_dp;
v2 player_half_size;

int number_of_life;

b32 first_ball_movement;
Ball balls[16];
int next_ball;

Block blocks[256];
int num_blocks;
int blocks_destroyed;

Level_State level_state;

b32 initialized = false;

f32 dt_multiplier = 1.f;

Level current_level;

#include "macros.c"

internal Block*
get_next_available_block() {
	Block *block = blocks+num_blocks++;
	if (num_blocks >= array_count(blocks)) num_blocks = 0;
	return block;
}

internal Ball*
get_next_available_ball() {
	for_each_ball {
		if (!(ball->flags & BALL_ACTIVE)) {
			zero_struct(*ball);
			return ball;
		}
	}
	invalid_code_path;
	return 0;
}

internal void
create_aligned_blocks(int num_x, int num_y, v2 block_half_size, f32 spacing_x, f32 spacing_y, f32 offset, f32 base_speed_multiplier) {
	f32 x_offset = (f32)num_x * block_half_size.x * (2.f+spacing_x) *.5f - block_half_size.x*(1.f+spacing_x/2.f);
	f32 y_offset = offset;
	for (int y = 0; y < num_y; ++y) {
		for (int x = 0; x < num_x; ++x) {
			Block *block = get_next_available_block();
			block->life = 1;
			block->half_size = block_half_size;
			block->relative_p.x = x*block->half_size.x*(2.f+spacing_x) - x_offset;
			block->relative_p.y = y*block->half_size.y*(2.f+spacing_y) - y_offset;

			if (current_level == L03_RIVALS) {
				if (y % 2) {
					block->color = 0x66ffff;
					block->flags = BLOCK_RIVAL_A;
					block->ball_speed_multiplier = 1+ (f32)y*1.1f/(f32)num_y;
				}
				else {
					block->flags = BLOCK_RIVAL_B;
					block->color = 0xffd366;
					block->ball_speed_multiplier = 1+ (f32)y*1.1f/(f32)num_y;
				}
			}
			else {
				u8 k = y*255/num_y;
				block->color = make_color_from_rgb(k, 255, 128);
				block->flags = BLOCK_RIVAL_A;
				block->ball_speed_multiplier = base_speed_multiplier + (f32)y*1.25f/(f32)num_y;
			}
		}
	}
}

internal void
create_invaders(v2 p, v2 block_half_size, f32 base_speed_multiplier) {
	char *invader[] = {"  0 0  ",
					   " 0 0 0 ",
					   " 00000 ",
					   "00   00",
					   "  0 0  "};

	p.x -= block_half_size.x * array_count(invader[0]);
	f32 original_x = p.x;

	for (unsigned int i = 0; i < array_count(invader); ++i) {
		char *at = invader[i];
		while(*at) {
			if (*at++ != ' '){
				Block *block = get_next_available_block();
				block->life = 1;
				block->half_size = block_half_size;
				block->relative_p = p;
				block->color = 0xffffff;
				block->ball_speed_multiplier = base_speed_multiplier + (f32)(array_count(invader)-i)*.75f/(f32)(array_count(invader));
				block->flags = BLOCK_RIVAL_A;
			}
			p.x += block_half_size.x*2.f;
		}
		p.y -= block_half_size.y*2.f;
		p.x = original_x;
	}
}

internal void
reset_ball_coll() {
	invicibility_time = 0.f;
	number_of_comet = 0;
	is_comet = false;
	number_of_triple_shots = 0;
	strong_blocks_time = 0.f;
	reverse_time = 0.f;
	slow_player_t = 0.f;

	balls[0].base_speed = 50;
	balls[0].dp.x = 0;
	balls[0].dp.y = -balls[0].base_speed;
	balls[0].p.x = 0;
	balls[0].p.y = 40;
	balls[0].half_size = (v2){.75, .75};
	balls[0].speed_multiplier = 1.f;
	balls[0].desired_p = (v2){0.f, 0.f};
	balls[0].color = 0x66bbff;
	balls[0].flags |= BALL_ACTIVE | BALL_RIVAL_A;

	if (current_level == L03_RIVALS) {
		balls[0].dp.x = 3;

		balls[1].base_speed = 50;
		balls[1].dp.x = -3;
		balls[1].dp.y = -balls[0].base_speed;
		balls[1].p.x = 0;
		balls[1].p.y = 80;
		balls[1].half_size = (v2){.75, .75};
		balls[1].speed_multiplier = 1.f;
		balls[1].desired_p = (v2){0.f, 0.f};
		balls[1].color = 0xffaa66;
		balls[1].flags |= BALL_ACTIVE | BALL_RIVAL_B;
	}
	first_ball_movement = true;
}

internal void
simulate_level(Level level, f32 dt) {
	switch(level) {
		case L04_PONG: {

			Level_Pong_State *pong = &level_state.pong;
			v2 ddp = mul_v2(pong->dp, -1.f); // friction

			if (balls[0].p.x > pong->p.x) ddp.x += 50.f;
			else if (balls[0].p.x < pong->p.x) ddp.x -= 50.f;
			v2 desired_dp = add_v2(pong->dp, mul_v2(ddp, dt));
			v2 desired_p = add_v2(add_v2(pong->p, mul_v2(desired_dp, dt)), 
								  mul_v2(ddp, square(dt)));
			if (desired_p.x - pong->half_size.x < -arena_half_size.x) {
				desired_p.x = -arena_half_size.x + pong->half_size.x;
				desired_dp.x = -desired_dp.x;
			}
			else if (desired_p.x + pong->half_size.x > arena_half_size.x) {
				desired_p.x = arena_half_size.x - pong->half_size.x;
				desired_dp.x = -desired_dp.x;
			}
			pong->dp = desired_dp;
			pong->p = desired_p;
		} break;
	}
}

internal void
simulate_block_for_level(Block *block, Level level) {
	switch(level) {
		case L04_PONG: {
			block->p = add_v2(block->relative_p, level_state.pong.p);
		} break;

		default: {
			block->p = block->relative_p;
		}
	}
}

internal void
restart_game() {

	if (current_level >= L_COUNT) current_level = 0;
	else if (current_level < 0) current_level = L_COUNT-1;

	zero_array(balls);
	zero_array(blocks);
	zero_array(colls);

	zero_struct(level_state);

	number_of_life = 3;

	reset_ball_coll();

	player_p.x = 0;
	player_p.y = -35;
	player_desired_p = (v2){0, -35};
	player_half_size = (v2){10, 2};

	coll_half_size = (v2){2, 2};
	invicibility_time = 0.f;

	arena_half_size = (v2){85, 45};

	num_blocks = 0;
	blocks_destroyed = 0;

	switch(current_level) {
		case L01_NORMAL: {
			create_aligned_blocks(18, 9, (v2){4, 2}, .2f, .4f, -4.f, 1.f);
		} break;

		case L02_WALL: {
			int num_x = 15;
			int num_y = 9;
			v2 block_half_size = (v2){4, 2};
			f32 x_offset = ((f32)num_x * block_half_size.x * (2.5f)) * .5f - block_half_size.x*(1.f+.5f)*.5f;
			f32 y_offset = -1.f;
			unsigned int collectible = 0;
			for (int y = 0; y < num_y; ++y) {
				for (int x = 0; x < num_x; ++x) {
					Block *block = get_next_available_block();

					block->life = 1;
					block->half_size = block_half_size;

					if (y%2) block->relative_p.x = x*block->half_size.x*2.5f - x_offset;
					else block->relative_p.x = x*block->half_size.x*2.5f - x_offset + block->half_size.x;
					block->relative_p.y = y*block->half_size.y*2.5f - y_offset;
					block->color = make_color_from_grey(y*255/num_y);
					block->ball_speed_multiplier = 1.f + (f32)y*1.25f/(f32)num_y;
					block->flags |= BLOCK_RIVAL_A;

					if (y == 0) {
						if (x % 4 == 0) block->coll = collectible++;
					}
					else if (y == 6 || y == 7) {
						if (x % 5 == 0) block->coll = collectible++;
					}
					else block->coll = 0;

					if (collectible % COLL_COUNT == 0) collectible = 1;
				}
			}
		} break;

		case L03_RIVALS: {
			create_aligned_blocks(18, 9, (v2){4, 2}, .2f, .4f, -4.f, 1.f);
		} break;

		case L04_PONG: {
			create_aligned_blocks(10, 3, (v2){2, 2}, 0.05f, 0.05f, -30.f, 2.f);
			level_state.pong.half_size = (v2){(2*2 + 0.05f) * 10/2, (2*2 + 0.05f) * 3/2};
		} break;

		case L05_INVADERS: {

			for (unsigned int i = 0; i < 5; ++i) {
				for (unsigned int j = 0; j < 2; ++j) {
					create_invaders((v2){(20*i)+3, (20*j)+3}, (v2){1.f,1.f}, 1.f);

				}
			}
		} break;

		invalid_default_case;
	}
}

internal void
test_for_win_condition() {
	if (blocks_destroyed == num_blocks) restart_game(++current_level);
}

internal void
block_destroyed(Block *block) {
	++blocks_destroyed;

	if (block->coll) {
		spawn_collectible(block->p, block->coll);
	}

	test_for_win_condition();
}

internal void
spawn_triple_shot() {

	for (int i = 0; i < 2; ++i) {
		Ball *ball = get_next_available_ball();

		ball->base_speed = 50;
		ball->dp.x = (1-(i%2))*45.f;
		ball->dp.y = ball->base_speed;
		ball->p.x = player_p.x;
		ball->p.y = player_p.y + player_half_size.y;
		ball->half_size = balls[0].half_size;
		ball->speed_multiplier = balls[0].speed_multiplier;
		ball->desired_p = (v2){0.f, 0.f};
		ball->flags |= BALL_ACTIVE | BALL_DESTROYED_ON_DP_Y_DOWN | BALL_RIVAL_A;
	}
}

#include "collision.c"

internal void
simulate_game(Input *input, f32 dt, b32 *running) {

	dt *= dt_multiplier;

	if (!initialized) {
		initialized = true;
		Level current_level = 0;
		restart_game();
	}

	if (reverse_time > 0 && slow_player_t <= 0) player_desired_p.x = player_p.x - pixels_dp_to_world(input->mouse_dp).x;
	else if (reverse_time <= 0 && slow_player_t > 0) player_desired_p.x = player_p.x + .5f * pixels_dp_to_world(input->mouse_dp).x;
	else if (reverse_time > 0 && slow_player_t > 0) player_desired_p.x = player_p.x - .5f * pixels_dp_to_world(input->mouse_dp).x;
	else player_desired_p.x = player_p.x + pixels_dp_to_world(input->mouse_dp).x;


	if (is_player_colliding_right_arena()) player_desired_p.x = arena_half_size.x - player_half_size.x ;
	else if (is_player_colliding_left_arena()) player_desired_p.x = player_half_size.x - arena_half_size.x ;
	player_desired_p.y = player_p.y;

	for_each_ball {
		if (!(ball->flags & BALL_ACTIVE)) continue;
		ball->desired_p = add_v2(ball->p, mul_v2(ball->dp, dt));

		ball_colliding_player(ball);

		ball_colliding_arena(ball);

		// Ball reached end of area
		if (ball->desired_p.y - ball->half_size.y < -50) {
			if (!(ball->flags & BALL_DESTROYED_ON_DP_Y_DOWN)) {
				--number_of_life;
				if (!number_of_life)  restart_game(current_level);
				else {
					reset_ball_coll();
				}
			}
			else {
				ball->flags &= ~BALL_ACTIVE;
			}
		}
	}

	simulate_level(current_level, dt);

	clear_screen_and_draw_rect((v2){0, 0}, arena_half_size, 0x000066, 0x006666, invicibility_time, first_ball_movement);

	for_each_block {
		if (block->life <= 0) continue;

		simulate_block_for_level(block, current_level);
		draw_rect(block->p, block->half_size, block->color);

		if (!first_ball_movement) {
			for_each_ball {
				if (!(ball->flags & BALL_ACTIVE)) continue;
				if ((ball->flags & BALL_RIVAL_A && block->flags & BLOCK_RIVAL_A) ||
				    (ball->flags & BALL_RIVAL_B && block->flags & BLOCK_RIVAL_B)) {
				    ball_colliding_block(ball, block);
				}
			}
		}
	}

	for_each_coll {
		if (coll->kind == COLL_INACTIVE) continue;
		coll->p.y -= 15*dt;

		if (is_colliding(player_p, player_half_size, coll->p, coll_half_size)) {
			switch(coll->kind){
				case COLL_INVICIBILITY: {
					invicibility_time += 5.f;
				} break;

				case COLL_TRIPLESHOT: {
					++number_of_triple_shots;
				} break;

				case COLL_COMET: {
					++number_of_comet;
				} break;

				case COLL_LOOSE_LIFE: {
					--number_of_life;
					if (!number_of_life) restart_game(current_level);
				} break;

				case COLL_STRONG_BLOCKS: {
					strong_blocks_time += 5.f;
				} break;

				case COLL_REVERSE_CONTROL: {
					reverse_time += 5.f;
				} break;

				case COLL_SLOW_PLAYER: {
					slow_player_t += 5.f;
				} break;

				invalid_default_case;
			}
			coll->kind = COLL_INACTIVE;
			continue;
		}
		draw_rect(coll->p, coll_half_size, 0xffff99);
	}

	// Render ball
	for_each_ball {
		if (!(ball->flags & BALL_ACTIVE)) continue;
		ball->p = ball->desired_p;
		if (ball->flags & BALL_DESTROYED_ON_DP_Y_DOWN) draw_rect(ball->p, ball->half_size, 0xffffff);
		else if (is_comet) draw_rect(ball->p, ball->half_size, 0xff9999);
		else draw_rect(ball->p, ball->half_size, ball->color);
	}

	player_dp.x = (player_desired_p.x - player_p.x) / dt;
	player_p = player_desired_p;

	if (invicibility_time > 0.f) invicibility_time -= dt;
	if (strong_blocks_time > 0.f) strong_blocks_time -= dt;
	if (reverse_time > 0.f) reverse_time -= dt;
	if (slow_player_t > 0.f) slow_player_t -= dt;
	if (number_of_triple_shots > 0) draw_rect(player_p, player_half_size, 0xffffff);
	else if (number_of_comet > 0) draw_rect(player_p, player_half_size, 0xff9999);
	else if (reverse_time > 0) draw_rect(player_p, player_half_size, 0x7f00ff);
	else if (slow_player_t > 0) draw_rect(player_p, player_half_size, 0x489000);
	else draw_rect(player_p, player_half_size, 0x80ff00);

	// Check Win
	test_for_win_condition();



	if (pressed(BUTTON_ESC)) *running = false;

#if DEVELOPMENT
	if (pressed(BUTTON_LEFT)) restart_game(--current_level);
	if (pressed(BUTTON_RIGHT)) restart_game(++current_level);
	if (pressed(BUTTON_DOWN)) dt_multiplier = 10.f;
	if (released(BUTTON_DOWN)) dt_multiplier = 1.f;
	if (is_down(BUTTON_UP)) invicibility_time += dt;
#endif
}