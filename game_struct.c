#define BALL_ACTIVE 0x1
#define BALL_DESTROYED_ON_DP_Y_DOWN 0x2
#define BALL_RIVAL_A 0x4
#define BALL_RIVAL_B 0x8
#define BALL_FIXED_SPEED 0x10

struct {
	v2 half_size;
	f32 left_wall_visual_p;
	f32 left_wall_visual_dp;
	f32 right_wall_visual_p;
	f32 right_wall_visual_dp;
	f32 top_wall_visual_p;
	f32 top_wall_visual_dp;
	f32 bottom_wall_visual_p;

} typedef Arena;

struct {
	v2 p;
	v2 desired_p;
	v2 dp;
	v2 half_size;

	b32 is_twinkling;
	b32 twinkle;
	f32 twinkling_t;
	f32 twinkling_target;
	u32 twinkling_number;
} typedef Player;

struct {
	v2 p;
	f32 life;
} typedef Ball_Trail;

struct {
	v2 p;
	v2 dp;
	v2 half_size;
	b32 base_speed;
	f32 speed_multiplier;
	v2 collision_test_p;
	v2 desired_p;
	u32 color;
	u32 flags;

	Ball_Trail trails[128];
	int next_trail;
	f32 trail_spawner;
	f32 trail_spawner_t;
} typedef Ball;

#define BLOCK_RIVAL_A 0x1
#define BLOCK_RIVAL_B 0x2
#define BLOCK_ACTIVE 0x4

struct {
	v2 relative_p;
	v2 p;
	v2 half_size;
	f32 ball_speed_multiplier;
	int life;
	u32 color;
	u32 flags;
	Coll_Kind coll;
} typedef Block;

enum {
	L01_NORMAL,
	L02_WALL,
	L03_RIVALS,
	L04_PONG,
	L05_INVADERS,

	L_COUNT,
} typedef Level;

struct {
	v2 p;
	v2 dp;
	v2 half_size;
} typedef Level_Pong_State;

struct {
	v2 p;
	v2 max_p;
	f32 x_movement;
	f32 movement_t;
	f32 movement_target;
	b32 is_moving_right;
	b32 is_moving_down;
} typedef Level_Invader_State;

struct {
	union {
		Level_Pong_State pong;
		Level_Invader_State invader;
	};
} typedef Level_State;

Arena arena;

int number_of_life;
int score;
f32 combo_time;

Player player;

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