#include "utils.c"
#include "math.c"
#include "string.c"


#include "platform_common.c"

#include <SDL2/SDL.h>


struct {
	// Platform non-specific part
	int width, height;
	u32 *pixels;
} typedef Render_buffer;

global_variable Render_buffer render_buffer;
global_variable f32 current_time;

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#include "software_rendering.c"
#include "game.c"

// File IO

internal void
free_file(String s) {
	free(s.data);
}

internal String
read_file(char *file_path) {
	String result  = {0};

	SDL_RWops *file_handle = SDL_RWFromFile(file_path, "rb");
	if (file_path == NULL) assert(0);

	Sint64 file_size = SDL_RWsize(file_handle);
	result.size = (s64)file_size;
	result.data = (char *)malloc(result.size+1);

	Sint64 nb_read_total = 0, nb_read = 1;
	char* buf = result.data;
	while (nb_read_total < file_size && nb_read != 0){
		nb_read = SDL_RWread(file_handle, buf, 1, (file_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}

	SDL_RWclose(file_handle);
	if(nb_read_total != file_size) {
		free(result.data);
		assert(0);
	}

	return result;
}

int main (int argc, char *argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window;

	u32 random_seed = (u32)SDL_GetTicks();
	set_random_seed(random_seed);

	SDL_DisplayMode DM_User;
	SDL_GetCurrentDisplayMode(0, &DM_User);

	b32 new_window = false;

	int max_w = DM_User.w;
	int max_h = DM_User.h;

	int display_size_ind = 0;
	int display_sizes[DISPLAY_SIZE_COUNT][2] = {{1280, 720}, {1920, 1080}, {2560, 1440}};

	int width = display_sizes[display_size_ind][0];
	int height = display_sizes[display_size_ind][1];

	window = SDL_CreateWindow("Casse_brique",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width,
                               height,
                               SDL_WINDOW_BORDERLESS);

	render_buffer.width = width;
	render_buffer.height = height;
	render_buffer.pixels = (u32 *)malloc(sizeof(u32)*width*height);

	u32 window_id = SDL_GetWindowID(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, width, height);

	SDL_Texture *texture = SDL_CreateTexture(renderer, DM_User.format, SDL_TEXTUREACCESS_STREAMING, width, height);

	Input input = {0};

	Uint64 current_counter = SDL_GetPerformanceCounter();
	Uint64 last_counter = 0;
	f32 last_dt = 0.01666f;

	SDL_WarpMouseGlobal((int)(max_w/2), (int)(max_h/2));

	int mouse_dp_x, mouse_dp_y;
	SDL_GetRelativeMouseState(&mouse_dp_x, &mouse_dp_y);
	SDL_ShowCursor(SDL_DISABLE);

	while(running) {

		SDL_GetRelativeMouseState(&mouse_dp_x, &mouse_dp_y);
		for (int i = 0; i < BUTTON_COUNT; ++i){
			input.buttons[i].changed = false;
		}

		SDL_Event event;
		while(SDL_PollEvent(&event)) {

			switch(event.type) {

				case SDL_MOUSEMOTION: {
					if (!new_window) SDL_GetRelativeMouseState(&mouse_dp_x, &mouse_dp_y);
					else {
						new_window = false;
						SDL_GetRelativeMouseState(&mouse_dp_x, &mouse_dp_y);
						mouse_dp_x = 0;
						mouse_dp_y = 0;
					} 
					if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) SDL_WarpMouseInWindow(window, (int)(width/2), (int)(height/2));
					else {
						SDL_ShowCursor(SDL_ENABLE);
						last_dt = 0;
					}
				}

				case SDL_KEYDOWN: 
				case SDL_KEYUP: {
					SDL_Keycode vk_code = event.key.keysym.sym;
					b32 was_down = (b32)(event.key.state == SDL_RELEASED);
					b32 is_down = (b32)(event.key.state == SDL_PRESSED);

					process_button(SDLK_LEFT, BUTTON_LEFT);
					process_button(SDLK_RIGHT, BUTTON_RIGHT);
					process_button(SDLK_UP, BUTTON_UP);
					process_button(SDLK_DOWN, BUTTON_DOWN);
					process_button(SDLK_ESCAPE, BUTTON_ESC);
					process_button(SDLK_F5, BUTTON_F5);

				} break;

				default:
					break;
			}
		}

		if (input.buttons[BUTTON_F5].is_down && input.buttons[BUTTON_F5].changed) {

			display_size_ind = (display_size_ind + 1) % DISPLAY_SIZE_COUNT;
			int width = min(max_w, display_sizes[display_size_ind][0]);
			int height = min(max_h, display_sizes[display_size_ind][1]);
			if (width >= max_w || height >= max_h) display_size_ind = DISPLAY_SIZE_COUNT - 1;

			render_buffer.width = width;
			render_buffer.height = height;
			free(render_buffer.pixels);
			render_buffer.pixels = (u32 *)malloc(sizeof(u32)*width*height);

			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyTexture(texture);

			new_window = true;

			window = SDL_CreateWindow("Casse_brique",
	                             	  SDL_WINDOWPOS_CENTERED,
	                                  SDL_WINDOWPOS_CENTERED,
	                                  width,
	                                  height,
	                                  SDL_WINDOW_BORDERLESS);
			window_id = SDL_GetWindowID(window);

			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
			SDL_RenderSetLogicalSize(renderer, width, height);

			texture = SDL_CreateTexture(renderer, DM_User.format, SDL_TEXTUREACCESS_STREAMING, width, height);

			SDL_WarpMouseGlobal((int)(max_w/2), (int)(max_h/2));
			SDL_GetRelativeMouseState(&mouse_dp_x, &mouse_dp_y);
			mouse_dp_x = 0;
			mouse_dp_y = 0;
		}

		input.mouse_dp = (v2i){-mouse_dp_x, mouse_dp_y};

		//Simulation
		simulate_game(&input, last_dt, &running);

		//Render
		SDL_UpdateTexture(texture, NULL, render_buffer.pixels, sizeof(u32)*width);
		SDL_RenderClear(renderer);
		SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0.f, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);

		last_counter = current_counter;
		current_counter = SDL_GetPerformanceCounter();

		last_dt = min_f32(.1f, (f32)((current_counter - last_counter) / (f64)SDL_GetPerformanceFrequency()));
		current_time += last_dt;
	}

	free(render_buffer.pixels);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	SDL_Quit();

	return 0;

}
