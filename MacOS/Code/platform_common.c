global_variable b32 running = true;

struct {
	b32 is_down;
	b32 changed;
} typedef Button;

enum {
	DISPLAY_HD,
	DISPLAY_FHD,
	DISPLAY_QHD,

	DISPLAY_SIZE_COUNT,
};


enum {
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_ESC,
	BUTTON_F5,

	BUTTON_COUNT,
};

struct {
	v2i mouse_p;
	v2i mouse_dp;
	Button buttons[BUTTON_COUNT];
} typedef Input;

#define process_button(vk, b) \
if (vk_code == vk) {\
	input.buttons[b].changed = is_down != input.buttons[b].is_down;\
	input.buttons[b].is_down = is_down;\
}

#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)
#define is_down(b) (input->buttons[b].is_down)

// Platform services to the game

internal String read_file(char *file_path);
internal void free_file(String s);