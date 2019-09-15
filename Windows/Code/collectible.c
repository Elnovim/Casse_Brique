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
		ball->dp.x = (2*i-1)*45.f;
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
