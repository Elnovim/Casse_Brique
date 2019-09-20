global_variable b32 running = true;

///////////////
// IO input

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
	BUTTON_P,

	BUTTON_COUNT,
};

struct {
	v2i mouse_p;
	v2i mouse_dp;
	Button buttons[BUTTON_COUNT];
} typedef Input;

#define process_button(vk, b) \
if (vk_code == vk) {\
	input_game.buttons[b].changed = is_down != input_game.buttons[b].is_down;\
	input_game.buttons[b].is_down = is_down;\
}

#define pressed(b) (input_game.buttons[b].is_down && input_game.buttons[b].changed)
#define released(b) (!input_game.buttons[b].is_down && input_game.buttons[b].changed)
#define is_down(b) (input_game.buttons[b].is_down)

//////////////////
// Audio 

struct {
	int size; // buffer size in bytes
	int channel_count;
	int bytes_per_sample;
	int samples_per_second;
	int running_sample_index;

	s16 *samples;
	int samples_to_write;

} typedef Game_Sound_Buffer;

/////////////////////
// Rendering

struct {
	int width, height;
	u32 *pixels;

} typedef Render_buffer;


//////////////////////
// Platform services to the game

internal String read_file(char *file_path);
internal void free_file(String s);

#define OS_JOB_CALLBACK(name) void name(struct Os_Job_Queue *queue, void *data)
typedef OS_JOB_CALLBACK(Os_Job_Callback);
internal void os_add_job_to_queue(struct Os_Job_Queue *queue, Os_Job_Callback *callback, void *data);