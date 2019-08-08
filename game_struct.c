#define BALL_ACTIVE 0x1
#define BALL_DESTROYED_ON_DP_Y_DOWN 0x2
#define BALL_RIVAL_A 0x4
#define BALL_RIVAL_B 0x8

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
	v2 dp;
	v2 half_size;
	b32 base_speed;
	f32 speed_multiplier;
	v2 collision_test_p;
	v2 desired_p;
	u32 color;
	u32 flags;
} typedef Ball;

#define BLOCK_RIVAL_A 0x1
#define BLOCK_RIVAL_B 0x2

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