#if DEVELOPMENT
struct {
	int val;
	f32 timer;
} typedef Message;


Message messages[32];
int current_message;


internal void
print_int(int number) {
	Message *message = messages + current_message++;
	if (current_message >= array_count(messages)) current_message = 0;

	message->val = number;
	message->timer = 2.f;
}

internal void
draw_messages(f32 dt) {
	v2 p = mul_v2(arena.half_size, -1.f);
	for (unsigned int i = 0; i < array_count(messages); ++i) {
		Message *message = messages + i;
		if (message->timer <= 0.f) continue;
		message->timer -= dt;
		draw_number(message->val, p, 2.5f, 0xffffff);

		p.y += 3.f;
	}
}

#else

#define draw_messages(...)
#define print_int(...)

#endif