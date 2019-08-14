internal void
spawn_collectible(v2 p, Coll_Kind kind) {
	Coll *coll = colls + next_coll++;
	if (next_coll >= array_count(colls)) next_coll = 0;
	coll->p = p;
	coll->kind = kind;
}