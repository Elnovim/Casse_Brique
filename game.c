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
		ball_dp.y = -30;
		ball_p.x = 75;
		ball_p.y = 40;
		ball_half_size = (v2){.75, .75};

		player_p.y = -35;
		player_half_size = (v2){10, 2};

		arena_half_size = (v2){85, 45};

		block_half_size = (v2){4, 2};
	}

#define num_x 10
#define num_y 10
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
			block->color = make_color_from_grey((x+y)*2);
		}
	}

	f32 player_new_x = pixels_to_world(input->mouse).x;
	player_dp.x = (player_new_x - player_p.x)/dt;

	player_p.x = player_new_x;

	v2 ball_desired_p = add_v2(ball_p, mul_v2(ball_dp, dt));

	if (ball_dp.y < 0 && aabb_vs_aabb(player_p, player_half_size, ball_desired_p, ball_half_size)) {
		
		// Player collision with ball
		ball_dp.y *= -1;
		ball_dp.x += player_dp.x*.25f;
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

		do_block_vs_ball_collision(block, block_half_size, ball_p, ball_desired_p);

		draw_rect(block->p, block_half_size, block->color);
	}

	ball_p = ball_desired_p;

	draw_rect(ball_p, ball_half_size, 0x00ffff);
	draw_rect(player_p, player_half_size, 0x00ff00);
}