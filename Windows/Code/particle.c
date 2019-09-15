inline Particle*
spawn_particle(v2 p, f32 dp_scale, v2 half_size, f32 angle, f32 life, f32 life_d, u32 color) {
	Particle *particle = particles + next_particle++;
	if (next_particle >= array_count(particles)) next_particle = 0;

	particle->p = p;
	particle->dp = (v2){random_bilateral()*dp_scale, random_bilateral()*dp_scale};
	particle->half_size = half_size;
	particle->life = life;
	particle->life_d = life_d;
	particle->color = color;
	particle->angle = angle;
	return particle;
}

inline void
spawn_particle_explosion(unsigned int count, v2 p, f32 dp_scale, f32 base_size, f32 base_life, u32 color) {
	for (unsigned int i = 0; i < count; ++i) {
		base_size += random_bilateral()*.1f*base_size;
		base_life += random_bilateral()*.1f*base_size; //A VOIR
		Particle *particle = spawn_particle(p, dp_scale, (v2){base_size, base_size}, random_f32_in_range(0, 360), base_life, 1.f, color);
	}
}