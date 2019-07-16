v2 ball_p;
v2 ball_dp;
v2 ball_half_size;

v2 player_p;
v2 player_dp;
v2 player_half_size;

v2 block_half_size;

struct {
	v2 p;
	int life;
	u32 color;
} typedef Block;

Block blocks[256];
int next_block;

v2 arena_half_size;

b32 initialized = false;

#include "collision.c"

internal void
simulate_game(Input *input, f32 dt) {

	if (!initialized) {
		initialized = true;
		ball_dp.y = -50;
		ball_p.x = 75;
		ball_p.y = 40;
		ball_half_size = (v2){.75, .75};

		player_p.y = -35;
		player_half_size = (v2){10, 2};

		arena_half_size = (v2){85, 45};

		block_half_size = (v2){4, 2};

#define num_x 18
#define num_y 9
		f32 x_offset = block_half_size.x*num_x;
		f32 y_offset = block_half_size.y*num_y + player_p.y/2;
		for (int y = 0; y < num_y; ++y) {
			for (int x = 0; x < num_x; ++x) {
				Block *block = blocks+next_block++;
				if (next_block >= array_count(blocks)) {
					next_block = 0;
				}

				block->life = 1;
				block->p.x = x*block_half_size.x*2 - x_offset;
				block->p.y = y*block_half_size.y*2 - y_offset;
				block->color = make_color_from_grey((x+y)*5);
			}
		}
	}

	v2 player_desired_p;
	player_desired_p.x = pixels_to_world(input->mouse).x;
	player_desired_p.y = player_p.y;
	v2 ball_desired_p = add_v2(ball_p, mul_v2(ball_dp, dt));

	if (ball_dp.y < 0 && is_colliding(player_p, player_half_size, ball_desired_p, ball_half_size)) {
		
		// Player collision with ball
		ball_dp.y *= -1;
		ball_dp.x += clamp(-10, player_dp.x, 100);
		ball_desired_p.y = player_p.y + player_half_size.y;
	}
	else if (ball_desired_p.x + ball_half_size.x > arena_half_size.x) {
		ball_desired_p.x = arena_half_size.x - ball_half_size.x;
		ball_dp.x *= -1;
	}
	else if (ball_desired_p.x - ball_half_size.x < -arena_half_size.x) {
		ball_desired_p.x = -arena_half_size.x + ball_half_size.x;
		ball_dp.x *= -1;
	}

	if (ball_desired_p.y + ball_half_size.y > arena_half_size.y) {
		ball_desired_p.y = arena_half_size.y - ball_half_size.y;
		ball_dp.y *= -1;
	}

	clear_screen_and_draw_rect((v2){0, 0}, arena_half_size, 0xff0000, 0x551100);

	for (Block *block = blocks; block != blocks+array_count(blocks); block++) {
		if (!block->life) continue;

		// Ball/block collision
		{
			f32 diff = ball_desired_p.y - ball_p.y;
			if (diff != 0) {
				f32 collision_point;
				if (ball_dp.y > 0) collision_point = block->p.y - block_half_size.y - ball_half_size.y;
				else collision_point = block->p.y + block_half_size.y + ball_half_size.y;
				f32 t_y = (collision_point - ball_p.y) / diff;
				if (t_y >= 0.f && t_y <= 1.f) {
					f32 target_x = lerp(ball_p.x, t_y, ball_desired_p.x);
					if (target_x + ball_half_size.x > block->p.x - block_half_size.x &&
						target_x - ball_half_size.x < block->p.x + block_half_size.x) {
							ball_desired_p.y = lerp(ball_p.y, t_y, ball_desired_p.y);
							ball_dp.y *= -1;
							block->life--;
					}
				}
			}

			diff = ball_desired_p.x - ball_p.x;
			if (diff != 0) {
				f32 collision_point;
				if (ball_dp.x > 0) collision_point = block->p.x - block_half_size.x - ball_half_size.x;
				else collision_point = block->p.x + block_half_size.x + ball_half_size.x;
				f32 t_x = (collision_point - ball_p.x) / diff;
				if (t_x >= 0.f && t_x <= 1.f) {
					f32 target_y = lerp(ball_p.y, t_x, ball_desired_p.y);
					if (target_y + ball_half_size.y > block->p.y - block_half_size.y &&
						target_y - ball_half_size.y < block->p.y + block_half_size.y) {
							ball_desired_p.x = lerp(ball_p.x, t_x, ball_desired_p.x);
							ball_dp.x *= -1;
							block->life--;
					}
				}
			}
		}

		draw_rect(block->p, block_half_size, block->color);
	}

	ball_p = ball_desired_p;
	player_dp.x = (player_desired_p.x - player_p.x) / dt;
	player_p = player_desired_p;

	draw_rect(ball_p, ball_half_size, 0x00ffff);
	draw_rect(player_p, player_half_size, 0x00ff00);
}