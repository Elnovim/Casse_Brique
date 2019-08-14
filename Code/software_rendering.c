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

internal void
draw_progressive_transparent_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color) {

	x0 = clamp(0, x0, render_buffer.width);
	y0 = clamp(0, y0, render_buffer.height);
	x1 = clamp(0, x1, render_buffer.width);
	y1 = clamp(0, y1, render_buffer.height);

	f32 alpha = 1.f;

	f32 alpha_step = 1.f/(y1-y0);

	for (int y = y0; y < y1; ++y) {
		u32 *pixel = render_buffer.pixels + x0 + render_buffer.width*y;
		for (int x = x0; x < x1; ++x)
			*pixel++ = lerp_color(*pixel, alpha, color);
		alpha -= alpha_step;
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

	half_size.x *= aspect_multiplier * scale;
	half_size.y *= aspect_multiplier * scale;

	p.x *= aspect_multiplier * scale;
	p.y *= aspect_multiplier * scale;

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

	alpha = clampf(0, alpha, 1);
	// Convert the untis to pixel and call draw_rect_in_pixels
	f32 aspect_multiplier = calculate_aspect_multiplier();

	half_size.x *= aspect_multiplier * scale;
	half_size.y *= aspect_multiplier * scale;

	p.x *= aspect_multiplier * scale;
	p.y *= aspect_multiplier * scale;

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
clear_arena_screen(v2 p, f32 top, f32 left, f32 right, u32 color) {
	f32 aspect_multiplier = calculate_aspect_multiplier();

	top *= aspect_multiplier * scale;
	left *= aspect_multiplier * scale;
	right *= aspect_multiplier * scale;

	p.x *= aspect_multiplier * scale;
	p.y *= aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height * .5f;

	int x0 = (int)(p.x+left);
	int x1 = (int)(p.x+right);
	int y1 = (int)(p.y+top);

	draw_rect_in_pixels(x0, 0, x1, y1, color);
}

internal void
draw_arena_rects(v2 p, f32 bottom, f32 top, f32 left, f32 right, u32 color, f32 invisibility_time, b32 first_ball_movement) {
	f32 aspect_multiplier = calculate_aspect_multiplier();

	bottom *= aspect_multiplier * scale;
	top *= aspect_multiplier * scale;
	left *= aspect_multiplier * scale;
	right *= aspect_multiplier * scale;

	p.x *= aspect_multiplier * scale;
	p.y *= aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height * .5f;

	int x0 = (int)(p.x+left);
	int y0 = (int)(p.y+bottom);
	int x1 = (int)(p.x+right);
	int y1 = (int)(p.y+top);

	draw_rect_in_pixels(0, 0, x0, render_buffer.height, color);
	draw_rect_in_pixels(x1, 0, render_buffer.width, render_buffer.height, color);
	draw_rect_in_pixels(x0, y1, x1, render_buffer.height, color);
	if (invisibility_time > 0 || first_ball_movement) {
		draw_progressive_transparent_rect_in_pixels(x0, 0, x1, y0, 0xafdfdf);
	}
}

internal void
draw_rotated_transparent_rect(v2 p, v2 half_size, f32 angle, u32 color, f32 alpha) { //In degrees
    
    alpha = clampf(0, alpha, 1);
    
    angle = deg_to_rad(angle);
    
    f32 cos = cosf(angle);
    f32 sin = sinf(angle);
    
    v2 x_axis = (v2){cos, sin};
    v2 y_axis = (v2){-sin, cos};
    
    m2 rotation = (m2){ 
        x_axis.x, y_axis.x,
        x_axis.y, y_axis.y,
    };
    
    Rect2 rect = make_rect_center_half_size((v2){0, 0}, half_size);
    
    for (int i = 0; i < 4; i++) 
        rect.p[i] = mul_m2_v2(rotation, rect.p[i]);
    
    
    // Change to pixels
    
    f32 aspect_multiplier = calculate_aspect_multiplier();
    f32 s = aspect_multiplier * scale;
    
    m2 world_to_pixels_scale_transform = (m2){
        s, 0,
        0, s,
    };
    
    v2 position_v = (v2){(f32)render_buffer.width * .5f,
        (f32)render_buffer.height * .5f};
    
    v2i min_bound = (v2i){render_buffer.width, render_buffer.height};
    v2i max_bound = (v2i){0, 0};
    
    
    for (int i = 0; i < 4; i++) {
        rect.p[i] = add_v2(p, rect.p[i]);
        rect.p[i] = mul_m2_v2(world_to_pixels_scale_transform, rect.p[i]);
        rect.p[i] = add_v2(rect.p[i], position_v);
        
        int x_t = trunc_f(rect.p[i].x);
        int x_c = ceil_f(rect.p[i].x);
        int y_t = trunc_f(rect.p[i].y);
        int y_c = ceil_f(rect.p[i].y);
        
        if (x_t < min_bound.x) min_bound.x = x_t;
        if (x_c > max_bound.x) max_bound.x = x_c;
        if (y_t < min_bound.y) min_bound.y = y_t;
        if (y_c > max_bound.y) max_bound.y = y_c;
    }
    
    min_bound.x = clamp(0, min_bound.x, render_buffer.width);
    min_bound.y = clamp(0, min_bound.y, render_buffer.height);
    max_bound.x = clamp(0, max_bound.x, render_buffer.width);
    max_bound.y = clamp(0, max_bound.y, render_buffer.height);
    
    // In pixels
    
    v2 axis_1 = sub_v2(rect.p[1], rect.p[0]);
    v2 axis_2 = sub_v2(rect.p[0], rect.p[1]);
    v2 axis_3 = sub_v2(rect.p[3], rect.p[0]);
    v2 axis_4 = sub_v2(rect.p[0], rect.p[3]);
    
    for (int y = min_bound.y; y < max_bound.y; ++y) {
        u32 *pixel = render_buffer.pixels + min_bound.x + render_buffer.width*y;
        for (int x = min_bound.x; x < max_bound.x; ++x) {
            
            v2 pixel_p = (v2){(f32)x, (f32)y};
            
            v2 pixel_p_rel_1 = sub_v2(pixel_p, rect.p[0]);
            v2 pixel_p_rel_2 = sub_v2(pixel_p, rect.p[1]);
            v2 pixel_p_rel_3 = sub_v2(pixel_p, rect.p[3]);
            
            f32 proj_0 = dot_v2(pixel_p_rel_1, axis_1);
            f32 proj_1 = dot_v2(pixel_p_rel_2, axis_2);
            
            f32 proj_2 = dot_v2(pixel_p_rel_1, axis_3);
            f32 proj_3 = dot_v2(pixel_p_rel_3, axis_4);
            
            if (proj_0 >= 0 &&
                proj_1 >= 0 &&
                proj_2 >= 0 &&
                proj_3 >=  0) {
                *pixel = lerp_color(*pixel, alpha, color); //@Speed
            }
            
            pixel++;
        }
    }
}

internal void
draw_rect_subpixel(v2 p, v2 half_size, u32 color) {

	f32 aspect_multiplier = calculate_aspect_multiplier();

	half_size.x *= aspect_multiplier * scale;
	half_size.y *= aspect_multiplier * scale;

	p.x *= aspect_multiplier * scale;
	p.y *= aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height *.5f;

	f32 x0f = p.x-half_size.x + .5f;
	int x0 = (int)x0f;
	f32 x0_alpha = x0f - (f32)x0;

	f32 y0f = p.y-half_size.y + .5f;
	int y0 = (int)y0f;
	f32 y0_alpha = y0f - (f32)y0;

	f32 x1f = p.x+half_size.x + .5f;
	int x1 = (int)x1f;
	f32 x1_alpha = x1f - (f32)x1;

	f32 y1f = p.y+half_size.y + .5f;
	int y1 = (int)y1f;
	f32 y1_alpha = y1f - (f32)y1;	

	draw_transparent_rect_in_pixels(x0, y0+1, x0+1, y1, color, 1.f-x0_alpha);
	draw_transparent_rect_in_pixels(x1, y0+1, x1+1, y1, color, x1_alpha);

	draw_transparent_rect_in_pixels(x0+1, y0, x1, y0+1, color, 1.f-y0_alpha);
	draw_transparent_rect_in_pixels(x0+1, y1, x1, y1+1, color, y1_alpha);

	draw_rect_in_pixels(x0+1, y0+1, x1, y1, color);
}

// Bitmap
struct {
	u32 *pixels;
	int width, height;
	int n_frames;
} typedef Bitmap;

internal Bitmap
load_gif(char *path) {
	Bitmap result;
	String image = read_file(path);
	int n;
	int *delays;
	stbi_set_flip_vertically_on_load(1);
	result.pixels = (u32 *)stbi_load_gif_from_memory((void*)image.data, (int)image.size, &delays, &result.width, &result.height, &result.n_frames, &n, 4);
	u32 *pixel = result.pixels;
	for (int z = 0; z < result.n_frames; ++z) {
		for (int y = 0; y < result.height; ++y) {
			for (int x = 0; x < result.width; ++x) {
				u8 r = (u8)(*pixel & 0x0000ff);
				u8 g = (u8)((*pixel & 0x00ff00) >> 8);
				u8 b = (u8)((*pixel & 0xff0000) >> 16);
				u8 a = (u8)((*pixel & 0xff000000) >> 24 );
				*pixel++ = b | (g << 8) | (r << 16) | (a << 24);
			}
		}
	}

	free_file(image);
	return result;
}

internal void
draw_bitmap(Bitmap *bitmap, v2 p, v2 half_size, f32 frame, f32 alpha_multiplier){

	alpha_multiplier = clampf(0.f, alpha_multiplier, 1.f);

	f32 aspect_multiplier = calculate_aspect_multiplier();

	half_size.x *= aspect_multiplier * scale;
	half_size.y *= aspect_multiplier * scale;

	p.x *= aspect_multiplier * scale;
	p.y *= aspect_multiplier * scale;

	p.x += (f32)render_buffer.width * .5f;
	p.y += (f32)render_buffer.height * .5f;

	int x0 = (int)(p.x-half_size.x);
	int y0 = (int)(p.y-half_size.y);
	int x1 = (int)(p.x+half_size.x);
	int y1 = (int)(p.y+half_size.y);

	// In Pixels 

	x0 = clamp(0, x0, render_buffer.width);
	y0 = clamp(0, y0, render_buffer.height);
	x1 = clamp(0, x1, render_buffer.width);
	y1 = clamp(0, y1, render_buffer.height);

	int z = trunc_f(frame);

	int start_pixel_bitmap = z * (bitmap->width*bitmap->height);

	for (int y = y0; y < y1; ++y) {
		u32 *pixel = render_buffer.pixels + x0 + render_buffer.width*y;

		f32 v = map_into_range_normalized((f32)y0, (f32)y, (f32)y1);
		int pixel_y = (int)lerp(0.f, (f32)v, (f32)bitmap->height);
		for (int x = x0; x < x1; ++x) {
			u32 color = 0x0000ff;

			f32 u = map_into_range_normalized((f32)x0, (f32)x, (f32)x1);
			int pixel_x = (int)(lerp(0.f, (f32)u, (f32)bitmap->width));

			color = *(bitmap->pixels + start_pixel_bitmap + pixel_x + pixel_y*bitmap->width);
			f32 alpha = (f32)((color & 0xff000000) >> 24)/255.f*alpha_multiplier;

			*pixel++ = lerp_color(*pixel, alpha, color);
		}
	}
}
