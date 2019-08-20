#include "game_struct.c"
#include "macros.c"
#include "console.c"

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

inline Particle*
spawn_particle(v2 p, f32 dp_scale, v2 half_size, f32 angle, f32 life, f32 life_d, u32 color) {
	Particle *particle = particles + next_particle++;
	if (next_particle >= array_count(particles)) next_particle = 0;

	particle->p = p;
	particle->dp = (v2){random_bilateral()*dp_scale, random_bilateral()*dp_scale};
	particle->half_size = half_size;
	particle->life = life;
	particle->life_d = life_d;
	particle->color = color;
	particle->angle = angle;
	return particle;
}

inline void
spawn_particle_explosion(unsigned int count, v2 p, f32 dp_scale, f32 base_size, f32 base_life, u32 color) {
	for (unsigned int i = 0; i < count; ++i) {
		base_size += random_bilateral()*.1f*base_size;
		base_life += random_bilateral()*.1f*base_size; //A VOIR
		Particle *particle = spawn_particle(p, dp_scale, (v2){base_size, base_size}, random_f32_in_range(0, 360), base_life, 1.f, color);
	}
}

inline void
increase_ball_size(Ball *ball) {
	ball->half_size.x += .15f/ball->half_size.x;
	ball->half_size.y += .15f/ball->half_size.y;
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
			block->coll = 0;
			if (random_choice(15)) block->coll = random_int_in_range(1, 3);
			else if (random_choice(30)) block->coll = random_int_in_range(4, 7);

			if (current_level == L03_RIVALS) {
				if (y % 2) {
					block->color = 0x66ffff;
					block->flags = BLOCK_RIVAL_A | BLOCK_ACTIVE;
					block->ball_speed_multiplier = 1+ (f32)y*1.1f/(f32)num_y;
				}
				else {
					block->flags = BLOCK_RIVAL_B | BLOCK_ACTIVE;
					block->color = 0xffd366;
					block->ball_speed_multiplier = 1+ (f32)y*1.1f/(f32)num_y;
				}
			}
			else {
				u8 k = (u8)(y*255/num_y);
				block->color = make_color_from_rgb(k, 255, 128);
				block->flags = BLOCK_RIVAL_A| BLOCK_ACTIVE;
				block->ball_speed_multiplier = base_speed_multiplier + (f32)y*1.25f/(f32)num_y;
			}
		}
	}
}

internal f32
create_invaders(int num_x, int num_y, v2 block_half_size, f32 base_speed_multiplier) {
	char *invader[] = {"  0 0  ",
					   " 0 0 0 ",
					   " 00000 ",
					   "00   00",
					   "  0 0  "};

	int size_y = 5;
	int size_x = 7;

	f32 x_offset = ((f32)size_x / 2.f) * block_half_size.x * 2.f * (f32)num_x - block_half_size.x + (3.f * block_half_size.x * ((f32)num_x-1));
	f32 y_offset = -40.f;

	for (int i = 0; i < num_x; ++i) {
		for (int j = 0; j < num_y; ++j) {
			for (unsigned int p = 0; p < array_count(invader); ++p) {
				char *at = invader[p];
				f32 d = 0;
				while(*at) {
					if (*at++ != ' '){
						Block *block = get_next_available_block();
						block->life = 1;
						block->half_size = block_half_size;
						block->relative_p.x = block->half_size.x*2.f*d + (i*(size_x+3)*block->half_size.x*2) - x_offset;
						block->relative_p.y = -block->half_size.y*2.f*p - (j*(size_y+3)*block->half_size.y*2) - y_offset;
						block->color = 0x33ff33;
						block->ball_speed_multiplier = base_speed_multiplier + (f32)(array_count(invader)-p)*.75f/(f32)(array_count(invader));
						block->flags = BLOCK_RIVAL_A | BLOCK_ACTIVE;
						block->coll = 0;
						if (random_choice(15)) block->coll = random_int_in_range(1, 3);
						else if (random_choice(30)) block->coll = random_int_in_range(4, 7);
					}
					++d;
				}
			}
		}
	}
	return (num_x*size_x*block_half_size.x*2 + (num_x-1)*3*block_half_size.x*2)/2.f;
}

internal void
reset_ball_coll() {

	f32 middle_point_y = (arena.half_size.y + (player.p.y + player.half_size.y)) * .5f;
	invincibility_time = 0.f;
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
	balls[0].p.y = middle_point_y;
	balls[0].half_size = (v2){.75, .75};
	balls[0].speed_multiplier = 1.f;
	balls[0].desired_p = balls[0].p;
	balls[0].collision_test_p = balls[0].p;
	balls[0].color = 0x66bbff;
	balls[0].flags |= BALL_ACTIVE | BALL_RIVAL_A;
	balls[0].next_trail = 0;
	balls[0].trail_spawner = .005f;
	balls[0].trail_spawner_t = 0.f;

	if (current_level == L03_RIVALS) {
		balls[0].flags |= BALL_FIXED_SPEED;

		balls[1].base_speed = 50;
		balls[1].dp.x = 0;
		balls[1].dp.y = -balls[1].base_speed * .5f;
		balls[1].p.x = 0;
		balls[1].p.y = middle_point_y;
		balls[1].half_size = (v2){.75, .75};
		balls[1].speed_multiplier = 1.f;
		balls[1].desired_p = balls[1].p;
		balls[1].collision_test_p = balls[1].p;		
		balls[1].color = 0xffaa66;
		balls[1].flags |= BALL_ACTIVE | BALL_RIVAL_B | BALL_FIXED_SPEED;
		balls[1].next_trail = 0;
		balls[1].trail_spawner = .005f;
		balls[1].trail_spawner_t = 0.f;
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
			if (desired_p.x - pong->half_size.x < arena.left_wall_visual_p) {
				desired_p.x = arena.left_wall_visual_p + pong->half_size.x;
				desired_dp.x = -desired_dp.x;
			}
			else if (desired_p.x + pong->half_size.x > arena.right_wall_visual_p) {
				desired_p.x = arena.right_wall_visual_p - pong->half_size.x;
				desired_dp.x = -desired_dp.x;
			}
			pong->dp = desired_dp;
			pong->p = desired_p;
		} break;
		case L05_INVADERS: {
			Level_Invader_State *invader = &level_state.invader;

			invader->movement_t += dt;
			if (invader->movement_t >= invader->movement_target) {
				invader->movement_t -= invader->movement_target;
				if (invader->is_moving_down) {
					invader->is_moving_down = false;
					invader->p.y -= 3.f;
				}
				else {
					if (invader->is_moving_right) { 
						invader->p.x += invader->x_movement;
						if (invader->p.x + invader->x_movement > invader->max_p.x) {
							invader->is_moving_right = false;
							invader->is_moving_down = true;
						}
					}
					else {
						invader->p.x -= invader->x_movement;
						if (invader->p.x - invader->x_movement < -invader->max_p.x) {
							invader->is_moving_right = true;
							invader->is_moving_down = true;
						}

					}
				}
			}

		}
	}
}

internal void
simulate_block_for_level(Block *block, Level level) {
	switch(level) {
		case L04_PONG: {
			block->p = add_v2(block->relative_p, level_state.pong.p);
		} break;

		case L05_INVADERS: {
			block->p = add_v2(block->relative_p, level_state.invader.p);
		} break;

		default: {
			block->p = block->relative_p;
		}
	}
}

internal f32
calculate_speed_adjustement(Ball *ball) {
	f32 time_to_player = 1.f;
	f32 dist_to_player = ball->p.y - (player.p.y + player.half_size.y);
	f32 result = (dist_to_player / time_to_player) / ball->base_speed;

	return result;
}

internal void
process_ball_when_dp_y_down(Ball *ball) {
	if (ball->flags & BALL_FIXED_SPEED) {
		if (balls[0].dp.y < 0) {
			ball->dp.y = -ball->base_speed * calculate_speed_adjustement(ball);
		}
	}
}

internal void
restart_game(int level) {

	current_level = level;
	if (current_level >= L_COUNT) current_level = 0;
	else if (current_level < 0) current_level = L_COUNT-1;

	zero_array(balls);
	zero_array(blocks);
	zero_array(colls);
	zero_array(particles);

	next_ball = 0;
	next_coll = 0 ;
	next_particle = 0;

	zero_struct(level_state);

	reset_ball_coll();

	number_of_life = 3;
	score = 0;
	combo_time = 0.f;

	player.p = (v2){0, -35};
	player.dp = (v2){0, 0};
	player.desired_p = (v2){0, -35};
	player.base_half_size = (v2){10, 2};
	player.half_size = player.base_half_size;

	player.color = 0x80ff00;

	player.visual_p = player.p;
	player.visual_dp = player.dp;
	player.visual_ddp = (v2){0, 0};

	player.squeeze_factor = 0.f;
	player.squeeze_factor_d = 0.f;
	player.squeeze_factor_dd = 0.f;

	player.is_twinkling = false;
	player.twinkle = false;
	player.twinkling_t = 0.f;
	player.twinkling_target = .2f;
	player.twinkling_number = 10;

	invincibility_time = 0.f;

	arena.half_size = (v2){85, 45};
	arena.left_wall_visual_p = -arena.half_size.x;
	arena.left_wall_visual_dp = 0.f;
	arena.right_wall_visual_p = arena.half_size.x;
	arena.right_wall_visual_dp = 0.f;
	arena.top_wall_visual_p = arena.half_size.y;
	arena.top_wall_visual_dp = 0.f;
	arena.bottom_wall_visual_p = -arena.half_size.y;

	arena.arena_color = 0x000066;
	arena.wall_color = 0x006666;

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
					block->color = make_color_from_grey((u8)(y*255/num_y));
					block->ball_speed_multiplier = 1.f + (f32)y*1.25f/(f32)num_y;
					block->flags = BLOCK_RIVAL_A | BLOCK_ACTIVE;
					block->coll = 0;

					if (random_choice(15)) block->coll = random_int_in_range(1, 3);
					else if (random_choice(30)) block->coll = random_int_in_range(4, 7);
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
			f32 half_size_invaders;
			half_size_invaders = create_invaders(5, 2, (v2){1.25f,1.25f}, 1.f);
			f32 max_x = arena.half_size.x - half_size_invaders;
			level_state.invader.x_movement = max_x / 4.f;
			level_state.invader.movement_target = 1.f;
			level_state.invader.max_p = (v2){max_x, 0};
			level_state.invader.is_moving_right = true;
			level_state.invader.is_moving_down = false;
		} break;

		invalid_default_case;
	}
}

internal void
spawn_collectible(v2 p, Coll_Kind kind) {
	Coll *coll = colls + next_coll++;
	if (next_coll >= array_count(colls)) next_coll = 0;
	coll->p = p;
	coll->kind = kind;
	coll->half_size = (v2){3, 3};
	coll->frame_t = 0.f;
}

internal void
spawn_triple_shot() {

	for (int i = 0; i < 2; ++i) {
		Ball *ball = get_next_available_ball();

		ball->base_speed = 50;
		ball->dp.x = (1-(i%2))*45.f;
		ball->dp.y = ball->base_speed;
		ball->p.x = player.p.x;
		ball->p.y = player.p.y + player.half_size.y;
		ball->half_size = balls[0].half_size;
		ball->speed_multiplier = balls[0].speed_multiplier;
		ball->desired_p = ball->p;
		ball->collision_test_p = ball->p;
		ball->flags |= BALL_ACTIVE | BALL_DESTROYED_ON_DP_Y_DOWN | BALL_RIVAL_A;
		ball->next_trail = 0;
		ball->trail_spawner = .005f;
		ball->trail_spawner_t = 0.f;
	}
}

internal void
test_for_win_condition() {
	if (blocks_destroyed == num_blocks) restart_game(current_level+1);
}

internal void
block_destroyed(Block *block, v2 ball_p) {
	spawn_particle_explosion(20, block->p, 12.f, 1.5f, .15f, block->color);
	Particle *block_particle = spawn_particle(block->p, 0.f, block->half_size, 0.f, 1.f, 5.f, block->color);
	block_particle->dp = sub_v2(block->p, ball_p);

	block->flags &= ~BLOCK_ACTIVE;
	++blocks_destroyed;
	f32 delta = max_f32(0.01f, (current_time - combo_time));
	int time_bonus = (int)(.4f/delta);
	score += number_of_life + time_bonus;
	combo_time = current_time;

	if (block->coll) {
		spawn_collectible(block->p, block->coll);
	}

	test_for_win_condition();
}

#include "collision.c"

Bitmap bitmap;

internal void
simulate_game(Input *input, f64 dt, b32 *is_running) {

	dt *= dt_multiplier;

	if (!initialized) {
		initialized = true;
		current_level = 0;
		restart_game(current_level);

		bitmap_invincibility = load_gif("../Sprites/Animations/Powerups/invincibility.GIF");
		bitmap_triple_shoot = load_gif("../Sprites/Animations/Powerups/triple_shoot.GIF");
		bitmap_comet = load_gif("../Sprites/Animations/Powerups/comet.GIF");
		bitmap_loose_life = load_gif("../Sprites/Animations/Powerdowns/Loose_Life.GIF");
		bitmap_reverse = load_gif("../Sprites/Animations/Powerdowns/Reverse.GIF");
		bitmap_slow_player = load_gif("../Sprites/Animations/Powerdowns/Slow_player.GIF");
		bitmap_strong_block = load_gif("../Sprites/Animations/Powerdowns/Strong_block.GIF");
		bitmap_block_strong = load_gif("../Sprites/Blocks/Block_strong.GIF");
	}

	

	f32 speed_multiplier = 1.f;
	if (slow_player_t > 0) speed_multiplier = .5f;
	f32 mouse_world_dp_x = speed_multiplier * clampf(-10.f, pixels_dp_to_world(input->mouse_dp).x, 10.f);
	if (reverse_time > 0 ) player.desired_p.x = player.p.x - mouse_world_dp_x;
	else player.desired_p.x = player.p.x + mouse_world_dp_x;

	if (player.desired_p.x < arena.left_wall_visual_p + player.base_half_size.x) {
		player.squeeze_factor_d = (player.desired_p.x - arena.left_wall_visual_p - player.base_half_size.x) * -1.f;
		player.desired_p.x = arena.left_wall_visual_p + player.base_half_size.x;
		player.dp.x = 0.f;
	}
	else if (player.desired_p.x > arena.right_wall_visual_p - player.base_half_size.x) {
		player.squeeze_factor_d = (player.desired_p.x - arena.right_wall_visual_p + player.base_half_size.x) * 1.f;
		player.desired_p.x = arena.right_wall_visual_p - player.base_half_size.x;
		player.dp.x = 0.f;
	}

	player.desired_p.y = player.p.y;
	player.visual_p.y = player.p.y;

	player.squeeze_factor_dd = -100.f*player.squeeze_factor - 10.f*player.squeeze_factor_d;
	player.squeeze_factor_d += player.squeeze_factor_dd*dt;
	player.squeeze_factor += player.squeeze_factor_dd*square(dt)*.5f + player.squeeze_factor_d*dt;

	player.visual_ddp.x = 1500.f*(player.desired_p.x - player.visual_p.x) - 40.f * player.visual_dp.x;
	player.visual_dp = add_v2(player.visual_dp, mul_v2(player.visual_ddp, dt));
	player.visual_p = add_v2(player.visual_p, add_v2(mul_v2(player.visual_dp, dt),
													 mul_v2(player.visual_ddp, square(dt)*.5f)));

	player.half_size.x = player.base_half_size.x + (dt*.5f*absf(player.dp.x)) - player.squeeze_factor;
	player.half_size.y = max_f32(.5f, player.base_half_size.y - dt*0.01f*absf(player.dp.x) + player.squeeze_factor);


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
					player.is_twinkling = true;
					player.twinkle = true;
					player.twinkling_t = 0.f;
					player.twinkling_number = 10;
				}
			}
			else {
				ball->flags &= ~BALL_ACTIVE;
			}
		}

		ball->collision_test_p = ball->desired_p;
	}

	simulate_level(current_level, dt);
	clear_arena_screen((v2){0, 0}, arena.top_wall_visual_p, arena.left_wall_visual_p, arena.right_wall_visual_p, arena.arena_color);

	for_each_block {
		if (!(block->flags & BLOCK_ACTIVE)) continue;

		simulate_block_for_level(block, current_level);

		if (!first_ball_movement) {
			for_each_ball {
				if (!(ball->flags & BALL_ACTIVE)) continue;
				if ((ball->flags & BALL_RIVAL_A && block->flags & BLOCK_RIVAL_A) ||
				    (ball->flags & BALL_RIVAL_B && block->flags & BLOCK_RIVAL_B)) {
				    ball_colliding_block(ball, block);
				}
			}
		}

		if (strong_blocks_time > 0.f) draw_bitmap(&bitmap_block_strong, block->p, block->half_size, 0, 1.f);
		else draw_rect(block->p, block->half_size, block->color);
	}

	// Check Win
	test_for_win_condition();

	for_each_coll {
		if (coll->kind == COLL_INACTIVE) continue;
		coll->p.y -= 15*dt;

		if (aabb_vs_aabb(player.p, player.half_size, coll->p, coll->half_size)) {
			switch(coll->kind){
				case COLL_INVINCIBILITY: {
					invincibility_time += 5.f;
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
					else {
						player.is_twinkling = true;
						player.twinkle = true;
						player.twinkling_t = 0.f;
						player.twinkling_number = 10;
					}
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

		switch (coll->kind) {
			case COLL_INVINCIBILITY: {
					draw_bitmap(&bitmap_invincibility, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_invincibility.n_frames) coll->frame_t = 0.f;
				} break;

			case COLL_TRIPLESHOT: {
					draw_bitmap(&bitmap_triple_shoot, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_triple_shoot.n_frames) coll->frame_t = 0.f;
				} break;

			case COLL_COMET: {
					draw_bitmap(&bitmap_comet, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_comet.n_frames) coll->frame_t = 0.f;
				} break;
			case COLL_LOOSE_LIFE: {
					draw_bitmap(&bitmap_loose_life, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_loose_life.n_frames) coll->frame_t = 0.f;
				} break;

			case COLL_STRONG_BLOCKS: {
					draw_bitmap(&bitmap_strong_block, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_strong_block.n_frames) coll->frame_t = 0.f;
				} break;

			case COLL_REVERSE_CONTROL: {
					draw_bitmap(&bitmap_reverse, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_reverse.n_frames) coll->frame_t = 0.f;
				} break;

			case COLL_SLOW_PLAYER: {
					draw_bitmap(&bitmap_slow_player, coll->p, coll->half_size, coll->frame_t, 1.f);
					coll->frame_t = coll->frame_t + dt*15;
					if (coll->frame_t >= bitmap_slow_player.n_frames) coll->frame_t = 0.f;
				} break;

			default: {
				draw_rect(coll->p, coll->half_size, 0xffff99);
			}

		}
	}

	// Render particles
	for (unsigned int i = 0; i < array_count(particles); ++i) {
		Particle *p = particles + i;
		if (p->life <= 0.f) continue;

		p->life -= p->life_d*dt;
		p->p = add_v2(p->p, mul_v2(p->dp, dt));

		draw_rotated_transparent_rect(p->p, p->half_size, p->angle, p->color, p->life);
	}

	// Render ball
	for_each_ball {
		if (!(ball->flags & BALL_ACTIVE)) continue;

		u32 ball_color = ball->color;
		if (ball->flags & BALL_DESTROYED_ON_DP_Y_DOWN) ball_color = 0xffffff;
		else if (is_comet) ball_color = 0xff9999;


		ball->p = ball->desired_p;

		ball->trail_spawner_t += dt;

		if (ball->trail_spawner_t >= ball->trail_spawner) {
			f32 speed_t = map_into_range_normalized(2500, len_sq(ball->dp), 100000);
			ball->trail_spawner_t -= 50.f/len_sq(ball->dp);
			u32 color = lerp_color(0x00bbee, speed_t, 0x33ffff);
			f32 life = .32f;

			f32 angle = find_look_at_rotation(ball->dp, (v2){0, 1});
			Particle *p = spawn_particle(ball->p, 2.f, (v2){ball->half_size.x, ball->half_size.y}, angle, life, 1.f, color);
		}

		draw_rect(ball->p, ball->half_size, ball_color);
		ball->half_size.x -= dt*max(1.f, ball->half_size.x);
		if (ball->half_size.x < .75f) ball->half_size.x = .75;
		ball->half_size.y -= dt*max(1.f, ball->half_size.y);
		if (ball->half_size.y < .75f) ball->half_size.y = .75;
	}

	player.dp.x = (player.desired_p.x - player.visual_p.x) / dt;
	player.p = player.desired_p;

	if (invincibility_time > 0.f) invincibility_time -= dt;
	if (strong_blocks_time > 0.f) strong_blocks_time -= dt;
	if (reverse_time > 0.f) reverse_time -= dt;
	if (slow_player_t > 0.f) slow_player_t -= dt;
	player.color = 0x80ff00;
	if (number_of_triple_shots > 0) player.color = 0xffffff;
	else if (number_of_comet > 0) player.color = 0xff9999;
	else if (reverse_time > 0) player.color = 0x7f00ff;
	else if (slow_player_t > 0) player.color = 0x489000;
	if (!player.twinkle) draw_rect_subpixel(player.visual_p, player.half_size, player.color);

	if (player.is_twinkling) {
		player.twinkling_t += dt;
		if (player.twinkling_t >= player.twinkling_target) {
			player.twinkling_t -= player.twinkling_target;
			player.twinkle = !player.twinkle;
			--player.twinkling_number;
		}
		if (player.twinkling_number == 0) {
			player.twinkle = false;
			player.is_twinkling = false;
		}
	}

	f32 arena_left_wall_visual_ddp = 150.f*(-arena.half_size.x - arena.left_wall_visual_p) - 7.f * arena.left_wall_visual_dp;
	arena.left_wall_visual_dp += arena_left_wall_visual_ddp*dt;
	arena.left_wall_visual_p += arena_left_wall_visual_ddp*square(dt)*.5f + arena.left_wall_visual_dp*dt;

	f32 arena_right_wall_visual_ddp = 150.f*(arena.half_size.x - arena.right_wall_visual_p) - 7.f * arena.right_wall_visual_dp;
	arena.right_wall_visual_dp += arena_right_wall_visual_ddp*dt;
	arena.right_wall_visual_p += arena_right_wall_visual_ddp*square(dt)*.5f + arena.right_wall_visual_dp*dt;

	f32 arena_top_wall_visual_ddp = 150.f*(arena.half_size.y - arena.top_wall_visual_p) - 7.f * arena.top_wall_visual_dp;
	arena.top_wall_visual_dp += arena_top_wall_visual_ddp*dt;
	arena.top_wall_visual_p += arena_top_wall_visual_ddp*square(dt)*.5f + arena.top_wall_visual_dp*dt;

	draw_arena_rects((v2){0, 0}, arena.bottom_wall_visual_p, arena.top_wall_visual_p, arena.left_wall_visual_p, arena.right_wall_visual_p, arena.wall_color, invincibility_time, first_ball_movement);

	for (int i = 0; i < number_of_life; ++i)
		draw_rect((v2){-arena.half_size.x+i*2.5f, arena.half_size.y+2.5f}, (v2){1,1}, 0x00ffff);

	draw_number(score, (v2){arena.half_size.x-10.f, arena.half_size.y+2.5f}, 4.f, 0xffffff);
	
	if (pressed(BUTTON_ESC)) *is_running = false;

	if (pressed(BUTTON_LEFT)) restart_game(current_level-1);
	if (pressed(BUTTON_RIGHT)) restart_game(current_level+1);

#if DEVELOPMENT
	if (pressed(BUTTON_LEFT)) restart_game(current_level-1);
	if (pressed(BUTTON_RIGHT)) restart_game(current_level+1);
	if (pressed(BUTTON_DOWN)) dt_multiplier = 10.f;
	if (released(BUTTON_DOWN)) dt_multiplier = 1.f;
	if (is_down(BUTTON_UP)) invincibility_time += dt;
	draw_number((int)(1.f/dt), (v2){0, arena.half_size.y+2.5f}, 4.f, 0xffffff);
#endif

	draw_messages(dt);
}