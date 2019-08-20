// Overlapping
inline b32
aabb_vs_aabb(v2 p1, v2 half_size1, v2 p2, v2 half_size2) {
	return p1.y + half_size1.y > p2.y - half_size2.y &&
		   p1.y - half_size1.y < p2.y + half_size2.y &&
		   p1.x + half_size1.x > p2.x - half_size2.x &&
		   p1.x - half_size1.x < p2.x + half_size2.x;
}

inline void
ball_colliding_arena(Ball *ball) {
	if (ball->desired_p.x + ball->half_size.x > arena.right_wall_visual_p) {
		increase_ball_size(ball);
		ball->desired_p.x = arena.right_wall_visual_p - ball->half_size.x;
		ball->dp.x *= -1;
		arena.right_wall_visual_dp = 10.f;
		spawn_particle_explosion(10, ball->desired_p, 8.f, 1.f, .3f, arena.wall_color);
	}
	else if (ball->desired_p.x - ball->half_size.x < arena.left_wall_visual_p) {
		increase_ball_size(ball);
		ball->desired_p.x = arena.left_wall_visual_p + ball->half_size.x;
		ball->dp.x *= -1;
		arena.left_wall_visual_dp = -10.f;
		spawn_particle_explosion(10, ball->desired_p, 8.f, 1.f, .3f, arena.wall_color);
	}

	if (ball->desired_p.y + ball->half_size.y > arena.top_wall_visual_p) {
		increase_ball_size(ball);
		ball->desired_p.y = arena.top_wall_visual_p - ball->half_size.y;
		ball->dp.y *= -1;
		arena.top_wall_visual_dp = 10.f;
		process_ball_when_dp_y_down(ball);
		if (is_comet) is_comet = false;
		spawn_particle_explosion(10, ball->desired_p, 8.f, 1.f, .3f, arena.wall_color);
	}

	if (first_ball_movement || invincibility_time > 0.f) {
		if (ball->desired_p.y - ball->half_size.y < -arena.half_size.y) {
			ball->desired_p.y = -arena.half_size.y + ball->half_size.y;
			ball->dp.y *= -1;
		}
	}
}

inline void
ball_colliding_player(Ball *ball) {
	if (ball->dp.y < 0 && aabb_vs_aabb(player.visual_p, player.half_size, ball->desired_p, ball->half_size)) {
		increase_ball_size(ball);
		first_ball_movement = false;
		// Player collision with ball
		ball->dp.y *= -1;
		ball->dp.x = (ball->p.x - player.visual_p.x)*7.5f;
		ball->dp.x += clamp(-25, (int)(player.dp.x*.5f), 25);
		ball->dp.x = (ball->p.x - player.visual_p.x)*7.5f;
		ball->desired_p.y = player.visual_p.y + player.half_size.y;
		spawn_particle_explosion(10, ball->desired_p, 8.f, 1.f, .3f, player.color);

		if (number_of_triple_shots) {
			--number_of_triple_shots;
			spawn_triple_shot();
		}
		if (number_of_comet > 0) {
			--number_of_comet;
			is_comet = true;
		}
	}
}

inline void
ball_colliding_block(Ball *ball, Block *block) {
	f32 diff = ball->collision_test_p.y - ball->p.y;
	if (diff != 0) {
		f32 collision_point;
		if (ball->dp.y > 0) collision_point = block->p.y - block->half_size.y - ball->half_size.y;
		else collision_point = block->p.y + block->half_size.y + ball->half_size.y;
		f32 t_y = (collision_point - ball->p.y) / diff;
		if (t_y >= 0.f && t_y <= 1.f) {
			f32 target_x = lerp(ball->p.x, t_y, ball->collision_test_p.x);
			if (target_x + ball->half_size.x > block->p.x - block->half_size.x &&
				target_x - ball->half_size.x < block->p.x + block->half_size.x) {
				increase_ball_size(ball);
				ball->desired_p.y = lerp(ball->p.y, t_y, ball->collision_test_p.y);
				if (block->ball_speed_multiplier > ball->speed_multiplier) ball->speed_multiplier = block->ball_speed_multiplier;
				if (ball->dp.y > 0) {
					if (ball->flags & BALL_DESTROYED_ON_DP_Y_DOWN) ball->flags &= ~BALL_ACTIVE;
					if (!is_comet) {
						ball->dp.y = -ball->base_speed * ball->speed_multiplier;
						process_ball_when_dp_y_down(ball);
					}
				}
				else ball->dp.y = ball->base_speed * ball->speed_multiplier;
				if (strong_blocks_time <= 0) {
					--block->life;
					if (block->life == 0) block_destroyed(block, ball->p);
				}
			}
		}
	}

	diff = ball->collision_test_p.x - ball->p.x;
	if (diff != 0) {
		f32 collision_point;
		if (ball->dp.x > 0) collision_point = block->p.x - block->half_size.x - ball->half_size.x;
		else collision_point = block->p.x + block->half_size.x + ball->half_size.x;
		f32 t_x = (collision_point - ball->p.x) / diff;
		if (t_x >= 0.f && t_x <= 1.f) {
			f32 target_y = lerp(ball->p.y, t_x, ball->collision_test_p.y);
			if (target_y + ball->half_size.y > block->p.y - block->half_size.y &&
				target_y - ball->half_size.y < block->p.y + block->half_size.y) {
				increase_ball_size(ball);
				ball->desired_p.x = lerp(ball->p.x, t_x, ball->collision_test_p.x);
				if (!is_comet) ball->dp.x *= -1;
				if (block->ball_speed_multiplier > ball->speed_multiplier) ball->speed_multiplier = block->ball_speed_multiplier;
				if (ball->dp.y > 0) {
					if (ball->flags & BALL_DESTROYED_ON_DP_Y_DOWN) ball->flags &= ~BALL_ACTIVE;
					if (!is_comet) ball->dp.y = ball->base_speed * ball->speed_multiplier;
				}
				else ball->dp.y = -ball->base_speed * ball->speed_multiplier;
				if (strong_blocks_time <= 0) {
					--block->life;
					if (block->life == 0) block_destroyed(block, ball->p);
				}
			}
		}
	}
}