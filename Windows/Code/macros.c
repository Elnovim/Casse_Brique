#define for_each_ball for (Ball *ball = balls; ball != balls + array_count(balls); ++ball)
#define for_each_block for (Block *block = blocks; block != blocks+array_count(blocks); block++)
#define for_each_coll for (Coll *coll = colls; coll != colls+array_count(colls); coll++)
#define for_each_ball_trail for (Ball_Trail *ball_trail = ball->trails; ball_trail != ball->trails + array_count(ball->trails); ++ball_trail)