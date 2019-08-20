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

	u32 arena_color;
	u32 wall_color;

} typedef Arena;

struct {
	v2 p;
	v2 desired_p;
	v2 dp;
	v2 half_size;
	v2 base_half_size;

	u32 color;

	v2 visual_p;
	v2 visual_dp;
	v2 visual_ddp;

	f32 squeeze_factor;
	f32 squeeze_factor_d;
	f32 squeeze_factor_dd;

	b32 is_twinkling;
	b32 twinkle;
	f32 twinkling_t;
	f32 twinkling_target;
	u32 twinkling_number;
} typedef Player;

struct {
	v2 p;
	v2 dp;
	v2 half_size;
	f32 angle;

	u32 color;
	f32 life;
	f32 life_d;
} typedef Particle;

struct {
	v2 p;
	v2 dp;
	v2 half_size;
	f32 base_speed;
	f32 speed_multiplier;
	v2 collision_test_p;
	v2 desired_p;
	u32 color;
	u32 flags;

	int next_trail;
	f32 trail_spawner;
	f32 trail_spawner_t;
} typedef Ball;

enum {
	COLL_INACTIVE,

	//Collups
	COLL_INVINCIBILITY,
	COLL_TRIPLESHOT,
	COLL_COMET,

	//Colldowns
	COLL_LOOSE_LIFE,
	COLL_STRONG_BLOCKS,
	COLL_REVERSE_CONTROL,
	COLL_SLOW_PLAYER,

	COLL_COUNT,
} typedef Coll_Kind;

struct {
	Coll_Kind kind;
	v2 p;
	v2 half_size;

	f32 frame_t;
} typedef Coll;

Bitmap bitmap_invincibility, bitmap_triple_shoot, bitmap_comet;
Bitmap bitmap_loose_life, bitmap_strong_block, bitmap_reverse, bitmap_slow_player;

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

Bitmap bitmap_block_strong;

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

f32 invincibility_time;
f32 strong_blocks_time;
f32 reverse_time;
f32 slow_player_t;

int number_of_triple_shots;
int number_of_comet;
b32 is_comet;

Coll colls[16];
int next_coll;

Particle particles[1024];
int next_particle;

Block blocks[256];
int num_blocks;
int blocks_destroyed;

Level_State level_state;

b32 initialized = false;

f32 dt_multiplier = 1.f;

Level current_level;