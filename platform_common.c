struct {
	b32 is_down;
	b32 changed;
} typedef Button;

enum {
	DISPLAY_QHD,
	DISPLAY_FHD,
	DISPLAY_HD,

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

#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)
#define is_down(b) (input->buttons[b].is_down)