internal void
clear_screen(u32 color){
	u32 *pixel = render_buffer.pixels;

	for (int y = 0; y < render_buffer.height; ++y) {
		for (int x = 0; x < render_buffer.width; ++x)
			*pixel++ = color;
	}
}

internal void
draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) { 

	x0 = clamp(0, x0, render_buffer.width);
	y0 = clamp(0, y0, render_buffer.height);
	x1 = clamp(0, x1, render_buffer.width);
	y1 = clamp(0, y1, render_buffer.height);

	for (int y = y0; y < y1; ++y) {
		u32 *pixel = render_buffer.pixels + x0 + render_buffer.width*y;
		for (int x = x0; x < x1; ++x)
			*pixel++ = color;
	}
}

internal void
draw_transparent_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color, f32 alpha) { 

	x0 = clamp(0, x0, render_buffer.width);
	y0 = clamp(0, y0, render_buffer.height);
	x1 = clamp(0, x1, render_buffer.width);
	y1 = clamp(0, y1, render_buffer.height);

	for (int y = y0; y < y1; ++y) {
		u32 *pixel = render_buffer.pixels + x0 + render_buffer.width*y;
		for (int x = x0; x < x1; ++x)
			*pixel++ = lerp_color(*pixel, alpha, color);
	}
}


global_variable f32 scale = 0.01f; //Hardcoded

inline f32
calculate_aspect_multiplier() {
	//Cleanup only needs to be done when size changes
	f32 aspect_multiplier = (f32)render_buffer.height;

	if ((f32)render_buffer.width / (f32)render_buffer.height < 1.77f) 
		aspect_multiplier = (f32)render_buffer.width / 1.77f;

	return aspect_multiplier;
}

internal v2
pixels_dp_to_world(v2i pixels_coord) {
	f32 aspect_multiplier = calculate_aspect_multiplier();

	v2 result;
	result.x = (f32)pixels_coord.x;
	result.y = (f32)pixels_coord.y;

	result.x /= aspect_multiplier;
	result.y /= aspect_multiplier;

	result.x /= scale;
	result.y /= scale;
	
	return result;
}

internal v2
pixels_to_world(v2i pixels_coord) {
	f32 aspect_multiplier = calculate_aspect_multiplier();

	v2 result;
	result.x = (f32)pixels_coord.x - (f32)render_buffer.width*.5f;
	result.y = (f32)pixels_coord.y - (f32)render_buffer.height*.5f;

	result.x /= aspect_multiplier;
	result.y /= aspect_multiplier;

	result.x /= scale;
	result.y /= scale;
	
	return result;
}

internal void
draw_rect(v2 p, v2 half_size, u32 color) {
	// Convert the untis to pixel and call draw_rect_in_pixels
	f32 aspect_multiplier = calculate_aspect_multiplier();

	half_size.x *= (f32)aspect_multiplier * scale;
	half_size.y *= (f32)aspect_multiplier * scale;

	p.x *= (f32)aspect_multiplier * scale;
	p.y *= (f32)aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height * .5f;

	int x0 = (int)(p.x-half_size.x);
	int y0 = (int)(p.y-half_size.y);
	int x1 = (int)(p.x+half_size.x);
	int y1 = (int)(p.y+half_size.y);

	draw_rect_in_pixels(x0, y0, x1, y1, color);
}

internal void
draw_transparent_rect(v2 p, v2 half_size, u32 color, f32 alpha) {
	// Convert the untis to pixel and call draw_rect_in_pixels
	f32 aspect_multiplier = calculate_aspect_multiplier();

	half_size.x *= (f32)aspect_multiplier * scale;
	half_size.y *= (f32)aspect_multiplier * scale;

	p.x *= (f32)aspect_multiplier * scale;
	p.y *= (f32)aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height * .5f;

	int x0 = (int)(p.x-half_size.x);
	int y0 = (int)(p.y-half_size.y);
	int x1 = (int)(p.x+half_size.x);
	int y1 = (int)(p.y+half_size.y);

	draw_transparent_rect_in_pixels(x0, y0, x1, y1, color, alpha);
}

internal void
draw_number(int number, v2 p, f32 size, u32 color) {
	int digit = number % 10;
	b32 first_digit = true;

	f32 square_size = size / 5.f;
	f32 half_square_size = size / 10.f;

	while (number || first_digit) {
		first_digit = false;

		switch(digit) {
			case 0: {
				draw_rect((v2){p.x-square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x+square_size, p.y},     (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x, p.y+square_size*2.f}, (v2){half_square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y-square_size*2.f}, (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 1: {
				draw_rect((v2){p.x+square_size, p.y}, (v2){half_square_size, 2.5f*square_size}, color);
				p.x -= square_size*2.f;
			} break;
			case 2: {
				draw_rect((v2){p.x, p.y+square_size*2.f},         (v2){1.5f*square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y},                         (v2){1.5f*square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y-square_size*2.f},         (v2){1.5f*square_size, half_square_size}, color);
				draw_rect((v2){p.x+square_size, p.y+square_size}, (v2){half_square_size, half_square_size}, color);
				draw_rect((v2){p.x-square_size, p.y-square_size}, (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 3: {
				draw_rect((v2){p.x-half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x-half_square_size, p.y},                 (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x-half_square_size, p.y-square_size*2.f}, (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x+half_square_size, p.y},                 (v2){half_square_size, 2.5f*square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 4: {
				draw_rect((v2){p.x+square_size, p.y},             (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x-square_size, p.y+square_size}, (v2){half_square_size, 1.5f*square_size}, color);
				draw_rect((v2){p.x, p.y},                         (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 5: {
				draw_rect((v2){p.x, p.y+square_size*2.f},         (v2){1.5f*square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y},                         (v2){1.5f*square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y-square_size*2.f},         (v2){1.5f*square_size, half_square_size}, color);
				draw_rect((v2){p.x-square_size, p.y+square_size}, (v2){half_square_size, half_square_size}, color);
				draw_rect((v2){p.x+square_size, p.y-square_size}, (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 6: {
				draw_rect((v2){p.x+half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x+half_square_size, p.y},                 (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x+half_square_size, p.y-square_size*2.f}, (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x-square_size, p.y},                      (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x+square_size, p.y-square_size},          (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 7: {
				draw_rect((v2){p.x+square_size, p.y},                      (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x-half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 8: {
				draw_rect((v2){p.x-square_size, p.y},      (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x+half_square_size, p.y}, (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x, p.y+square_size*2.f},  (v2){half_square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y-square_size*2.f},  (v2){half_square_size, half_square_size}, color);
				draw_rect((v2){p.x, p.y},                  (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;
			case 9: {
				draw_rect((v2){p.x-half_square_size, p.y+square_size*2.f}, (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x-half_square_size, p.y},                 (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x-half_square_size, p.y-square_size*2.f}, (v2){square_size, half_square_size}, color);
				draw_rect((v2){p.x+square_size, p.y},                      (v2){half_square_size, 2.5f*square_size}, color);
				draw_rect((v2){p.x-square_size, p.y+square_size},          (v2){half_square_size, half_square_size}, color);
				p.x -= square_size*4.f;
			} break;

			invalid_default_case;
		}

		number /= 10;
		digit = number % 10;
	}
}

internal void
clear_screen_and_draw_rect(v2 p, f32 bottom, f32 top, f32 left, f32 right, u32 color, u32 clear_color, f32 invisibility_time, b32 first_ball_movement) {
	// Convert the untis to pixel and call draw_rect_in_pixels
	f32 aspect_multiplier = calculate_aspect_multiplier();

	left *= (f32)aspect_multiplier * scale;
	right *= (f32)aspect_multiplier * scale;
	top *= (f32)aspect_multiplier * scale;
	bottom *= (f32)aspect_multiplier * scale;

	p.x *= (f32)aspect_multiplier * scale;
	p.y *= (f32)aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height * .5f;

	int x0 = (int)(p.x+left);
	int y0 = (int)(p.y+bottom);
	int x1 = (int)(p.x+right);
	int y1 = (int)(p.y+top);

	if (invisibility_time > 0 || first_ball_movement){
		draw_rect_in_pixels(x0, y0, x1, y1, color);
		draw_rect_in_pixels(x0, 0, x1, y0, 0xafdfdf); //HardCoded

	}
	else draw_rect_in_pixels(x0, 0, x1, y1, color);

	draw_rect_in_pixels(0, 0, x0, render_buffer.height, clear_color);
	draw_rect_in_pixels(x1, 0, render_buffer.width, render_buffer.height, clear_color);
	draw_rect_in_pixels(x0, y1, x1, render_buffer.height, clear_color);
}