#include "utils.c"
#include "math.c"
#include "string.c"


#include "platform_common.c"

#include <windows.h>


struct {
	// Platform non-specific part
	int width, height;
	u32 *pixels;

	// Platform specific part
	BITMAPINFO bitmap_info;

} typedef Render_buffer;

global_variable Render_buffer render_buffer;
global_variable f32 current_time;

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#include "software_rendering.c"
#include "game.c"

f32 frequency_counter;
b32 pause;

///////////
// Time
inline f32
os_seconds_elapsed(u64 last_counter) {
    LARGE_INTEGER current_counter;
    QueryPerformanceCounter(&current_counter);
    return (f32)(current_counter.QuadPart - last_counter) / frequency_counter;
}

inline u64
os_get_perf_counter() {
    LARGE_INTEGER current_counter;
    QueryPerformanceCounter(&current_counter);
    return current_counter.QuadPart;
}

// File IO

internal void
free_file(String s) {
	VirtualFree(s.data, 0, MEM_RELEASE);
}

internal String
read_file(char *file_path) {
	String result  = {0};

	HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (file_path == INVALID_HANDLE_VALUE) {
		assert(0);
		return result;
	}

	DWORD file_size = GetFileSize(file_handle, 0);
	result.size = file_size;
	result.data = VirtualAlloc(0, result.size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

	DWORD bytes_read;

	if (ReadFile(file_handle, result.data, file_size, &bytes_read, 0) && file_size == bytes_read) {

	}
	else {
		assert(0);
	}

	return result;

}

LRESULT window_callback (HWND window, UINT message, WPARAM w_param, LPARAM l_param) {

	LRESULT result = 0;

	switch(message) {
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;
		} break;

		case WM_SIZE: {
			// Get width and height
			RECT rect;
			GetClientRect(window, &rect);
			render_buffer.width = rect.right - rect.left;
			render_buffer.height = rect.bottom - rect.top;

			// Allocate the buffer
			if (render_buffer.pixels) {
				VirtualFree(render_buffer.pixels, 0, MEM_RELEASE);
			}
			render_buffer.pixels = VirtualAlloc(0,
												sizeof(u32)*render_buffer.width*render_buffer.height,
												MEM_COMMIT|MEM_RESERVE,
												PAGE_READWRITE);

			// Fill the bitmap info
			render_buffer.bitmap_info.bmiHeader.biSize = sizeof(render_buffer.bitmap_info.bmiHeader);
			render_buffer.bitmap_info.bmiHeader.biWidth = render_buffer.width;
			render_buffer.bitmap_info.bmiHeader.biHeight = render_buffer.height;
			render_buffer.bitmap_info.bmiHeader.biPlanes = 1;
			render_buffer.bitmap_info.bmiHeader.biBitCount = 32;
			render_buffer.bitmap_info.bmiHeader.biCompression = BI_RGB;
		} break;

		default: {
			result = DefWindowProcA(window, message, w_param, l_param);
		}
	}
	return result;
}

int __stdcall WinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPSTR lpCmdLine,
					  int nShowCmd) {

	SYSTEMTIME st;
	GetSystemTime(&st);

	u32 random_seed = 31557600 * (st.wYear - 1 - 1970) + 2629800 * (st.wMonth - 1) + 86400 * (st.wDay - 1) + 3600 * st.wHour + 60 * st.wMinute + st.wSecond;
	set_random_seed(random_seed);

	int max_w = GetSystemMetrics(SM_CXFULLSCREEN);
	int max_h = GetSystemMetrics(SM_CYFULLSCREEN);

	WNDCLASSA window_class = {0};
	window_class.style = CS_HREDRAW|CS_VREDRAW;
	window_class.lpfnWndProc = (WNDPROC) window_callback;
	window_class.lpszClassName = "Game_Window_Class";

	RegisterClassA(&window_class);

	int display_size_ind = 0;
	int display_sizes[DISPLAY_SIZE_COUNT][2] = {{1280, 720}, {1920, 1080}, {2560, 1440}};

	int width = display_sizes[display_size_ind][0];
	int height = display_sizes[display_size_ind][1];

	HWND window = CreateWindowExA(0, // Windows Style
								   window_class.lpszClassName,
								   "CasseBrique",
								   WS_VISIBLE|WS_OVERLAPPEDWINDOW,
								   (int)((max_w-width)/2), 
							   	   (int)((max_h-height)/2),
								   display_sizes[display_size_ind][0], 
								   display_sizes[display_size_ind][1],
								   0,
								   0,
								   0,
								   0);

	HDC hdc = GetDC(window);
	SetFocus(window);

	Input input = {0};

	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);
	f32 last_dt = 0.01666f;
	f32 target_dt = 0.01666f;

	LARGE_INTEGER frequency_counter_l;
	QueryPerformanceFrequency(&frequency_counter_l);

	frequency_counter = (f32) frequency_counter_l.QuadPart;

	SetCursorPos(max_w / 2, max_h / 2);

	pause = false;

	while(running) {

		POINT mouse_pointer;
		GetCursorPos(&mouse_pointer);
		ShowCursor(false);
		//ScreenToClient(window, &mouse_pointer);
		//Input

		for (int i = 0; i < BUTTON_COUNT; ++i){
			input.buttons[i].changed = false;
		}

		MSG message;
		while(PeekMessageA(&message, window, 0, 0, PM_REMOVE)) {

			switch(message.message) {

				case WM_MOUSEMOVE: {
					GetCursorPos(&mouse_pointer);
					if (GetFocus() == window) {
						SetCursorPos(max_w / 2, max_h / 2);
						pause = false;
					} 
					else {
						ShowCursor(true);
						pause = true;
					}
				}

				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				case WM_KEYDOWN: 
				case WM_KEYUP: {

					u32 vk_code = (u32)message.wParam;
					b32 was_down = ((message.lParam & (1 << 30)) != 0);
					b32 is_down = ((message.lParam & (1 << 31)) == 0);



					process_button(VK_LEFT, BUTTON_LEFT);
					process_button(VK_RIGHT, BUTTON_RIGHT);
					process_button(VK_UP, BUTTON_UP);
					process_button(VK_DOWN, BUTTON_DOWN);
					process_button(VK_ESCAPE, BUTTON_ESC);
					process_button(VK_F5, BUTTON_F5);

				} break;

				default : {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
		}

		input.mouse_dp = sub_v2i((v2i){mouse_pointer.x, mouse_pointer.y}, (v2i){max_w / 2, max_h / 2});

		//Simulation
		if (!pause) simulate_game(&input, last_dt, &running);

		//Render
		StretchDIBits(hdc,
					  0,
					  0,
					  render_buffer.width,
					  render_buffer.height,
					  0,
					  0,
					  render_buffer.width,
					  render_buffer.height,
					  render_buffer.pixels,
					  &render_buffer.bitmap_info,
					  DIB_RGB_COLORS,
					  SRCCOPY);

		if (input.buttons[BUTTON_F5].is_down && input.buttons[BUTTON_F5].changed) {

			display_size_ind = (display_size_ind + 1) % DISPLAY_SIZE_COUNT;
			width = min(max_w, display_sizes[display_size_ind][0]);
			height = min(max_h, display_sizes[display_size_ind][1]);
			if (width == max_w || height == max_h) display_size_ind = DISPLAY_SIZE_COUNT - 1;
			SetWindowPos(window, HWND_TOP, (max_w-width)/2, (max_h-height)/2, width, height, SWP_SHOWWINDOW);
		}

		f32 dt_before_sleep = min(.1f, os_seconds_elapsed(last_counter.QuadPart));

		int sleep = (int)((target_dt-dt_before_sleep)*1000.f);
		if (sleep > 1) {
			Sleep(sleep-1);
		}

		last_dt = min(.1f, os_seconds_elapsed(last_counter.QuadPart));
		current_time += last_dt;
		QueryPerformanceCounter(&last_counter);
	}

}