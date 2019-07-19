// Overlapping
inline b32
is_colliding(v2 p1, v2 half_size1, v2 p2, v2 half_size2) {
	return p1.y + half_size1.y > p2.y - half_size2.y &&
		   p1.y - half_size1.y < p2.y + half_size2.y &&
		   p1.x + half_size1.x > p2.x - half_size2.x &&
		   p1.x - half_size1.x < p2.x + half_size2.x;
}

inline void
ball_colliding_arena(v2 *ball_desired_p) {
	if (ball_desired_p->x + ball_half_size.x > arena_half_size.x) {
		ball_desired_p->x = arena_half_size.x - ball_half_size.x;
		ball_dp.x *= -1;
	}
	else if (ball_desired_p->x - ball_half_size.x < -arena_half_size.x) {
		ball_desired_p->x = -arena_half_size.x + ball_half_size.x;
		ball_dp.x *= -1;
	}

	if (ball_desired_p->y + ball_half_size.y > arena_half_size.y) {
		ball_desired_p->y = arena_half_size.y - ball_half_size.y;
		ball_dp.y *= -1;
	}

	if (invicible) {
		if (ball_desired_p->y - ball_half_size.y < -arena_half_size.y) {
			ball_desired_p->y = -arena_half_size.y + ball_half_size.y;
			ball_dp.y *= -1;
		}
	}
}

inline void
ball_colliding_player(v2 *ball_desired_p) {
	if (ball_dp.y < 0 && is_colliding(player_p, player_half_size, *ball_desired_p, ball_half_size)) {
		first_ball_movement = false;
		// Player collision with ball
		ball_dp.y *= -1;
		ball_dp.x = (ball_p.x - player_p.x)*7.5f;
		ball_dp.x += clamp(-25, player_dp.x*.5f, 25);
		ball_dp.x = (ball_p.x - player_p.x)*7.5f;
		ball_desired_p->y = player_p.y + player_half_size.y;
	}
}

inline void
ball_colliding_block(v2 ball_desired_p, Block *block) {
	f32 diff = ball_desired_p.y - ball_p.y;
	if (diff != 0) {
		f32 collision_point;
		if (ball_dp.y > 0) collision_point = block->p.y - block->half_size.y - ball_half_size.y;
		else collision_point = block->p.y + block->half_size.y + ball_half_size.y;
		f32 t_y = (collision_point - ball_p.y) / diff;
		if (t_y >= 0.f && t_y <= 1.f) {
			f32 target_x = lerp(ball_p.x, t_y, ball_desired_p.x);
			if (target_x + ball_half_size.x > block->p.x - block->half_size.x &&
				target_x - ball_half_size.x < block->p.x + block->half_size.x) {
					ball_desired_p.y = lerp(ball_p.y, t_y, ball_desired_p.y);
					if (block->ball_speed_multiplier > ball_speed_multiplier) ball_speed_multiplier = block->ball_speed_multiplier;
					if (ball_dp.y > 0) ball_dp.y = -ball_base_speed * ball_speed_multiplier;
					else ball_dp.y = ball_base_speed * ball_speed_multiplier;
					--block->life;
					block_destroyed(block);
			}
		}
	}

	diff = ball_desired_p.x - ball_p.x;
	if (diff != 0) {
		f32 collision_point;
		if (ball_dp.x > 0) collision_point = block->p.x - block->half_size.x - ball_half_size.x;
		else collision_point = block->p.x + block->half_size.x + ball_half_size.x;
		f32 t_x = (collision_point - ball_p.x) / diff;
		if (t_x >= 0.f && t_x <= 1.f) {
			f32 target_y = lerp(ball_p.y, t_x, ball_desired_p.y);
			if (target_y + ball_half_size.y > block->p.y - block->half_size.y &&
				target_y - ball_half_size.y < block->p.y + block->half_size.y) {
					ball_desired_p.x = lerp(ball_p.x, t_x, ball_desired_p.x);
					ball_dp.x *= -1;
					if (block->ball_speed_multiplier > ball_speed_multiplier) ball_speed_multiplier = block->ball_speed_multiplier;
					if (ball_dp.y > 0) ball_dp.y = ball_base_speed * ball_speed_multiplier;
					else ball_dp.y = -ball_base_speed * ball_speed_multiplier;
					--block->life;
					block_destroyed(block);
			}
		}
	}
}