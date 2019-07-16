internal int
clamp(int min, int val, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

// Color
inline u32
make_color_from_grey(u8 grey) {
	return (grey << 0) | (grey << 8) | (grey << 16);
}

// Vector 2

struct {
	union {
		struct {
			f32 x;
			f32 y;
		};
		
		f32 e[2];
	};
} typedef v2;

inline v2
add_v2(v2 a, v2 b) {
	return (v2){a.x + b.x, a.y + b.y};
}

inline v2
sub_v2(v2 a, v2 b) {
	return (v2){a.x - b.x, a.y - b.y};
}

inline v2
mul_v2(v2 a, f32 s) {
	return (v2){s * a.x, s * a.y};
}

struct {
	union {
		struct {
			int x;
			int y;
		};
		
		int e[2];
	};
} typedef v2i;