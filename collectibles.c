enum {
	COLL_INACTIVE,

	//Collups
	COLL_INVICIBILITY,
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
} typedef Coll;

v2 coll_half_size;

f32 invicibility_time;
f32 strong_blocks_time;
f32 reverse_time;
f32 slow_player_t;

int number_of_triple_shots;
int number_of_comet;
b32 is_comet;

Coll colls[16];
int next_coll;

internal void
spawn_collectible(v2 p, Coll_Kind kind) {
	Coll *coll = colls + next_coll++;
	if (next_coll >= array_count(colls)) next_coll = 0;
	coll->p = p;
	coll->kind = kind;
}