#include "utils.c"
#include "math.c"
#include "string.c"
#include "platform_common.c"

#include <windows.h>

BITMAPINFO w32_bitmap_info;

global_variable Render_buffer render_buffer;
global_variable f32 current_time;

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#include "wav_importer.h"

#include "software_rendering.c"
#include "audio.c"
#include "game.c"

f32 frequency_counter;
b32 pause;

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

///////////////////////
// Multi-Threading

struct {
	void *data;
	Os_Job_Callback *callback;
} typedef Os_Job;

struct Os_Job_Queue{
	// Platform independent part
	u32 volatile next_entry_to_read;
	u32 volatile next_entry_to_write;
	Os_Job entries[32];

	// Platform dependent part
	HANDLE semaphore;
} typedef Os_Job_Queue;

internal b32
win32_do_next_queue_job(Os_Job_Queue *queue) {
	u32 volatile original_next = queue->next_entry_to_read;
	u32 volatile new_entry_to_read = (original_next+1) % array_count(queue->entries);

	if (original_next != queue->next_entry_to_write) {

		u32 interlocked_value = InterlockedCompareExchange((volatile long*)&queue->next_entry_to_read, new_entry_to_read, original_next);
		if (interlocked_value == original_next) {
			Os_Job *entry = queue->entries + interlocked_value;
			entry->callback(queue, entry->data);
			return false;
		}
	}

	return true;
}

DWORD WINAPI win32_thread_proc(void *data) {
	Os_Job_Queue *queue = (Os_Job_Queue*)data;

	for (;;) {
		if (win32_do_next_queue_job(queue)) {
			WaitForSingleObjectEx(queue->semaphore, INFINITE, FALSE);
		}
	}
}

internal void
win32_create_queue(Os_Job_Queue *queue, int number_of_thread) {

	zero_struct(*queue);
	queue->semaphore = CreateSemaphoreExA(0, 0, number_of_thread, 0, 0, SEMAPHORE_ALL_ACCESS);


	for (int i = 0; i < number_of_thread; ++i) {
		HANDLE thread = CreateThread(0, 0, win32_thread_proc, queue, 0, 0);
		CloseHandle(thread);
	}
}

internal void
os_add_job_to_queue(Os_Job_Queue *queue, Os_Job_Callback *callback, void *data) {
	u32 volatile new_next_entry_to_write =(queue->next_entry_to_write + 1)%array_count(queue->entries);
	assert(new_next_entry_to_write != queue->next_entry_to_read);

	Os_Job *entry = queue->entries + queue->next_entry_to_write;
	entry->callback = callback;
	entry->data = data;
	_WriteBarrier();
	queue->next_entry_to_write = new_next_entry_to_write;
	ReleaseSemaphore(queue->semaphore, 1, 0);
}

OS_JOB_CALLBACK(print_job) {
	print_int((int)data);
}

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


////////////////////////
// Audio

#include <dsound.h>

typedef HRESULT Direct_Sound_Create (LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);

global_variable LPDIRECTSOUNDBUFFER win32_sound_buffer;

global_variable Game_Sound_Buffer sound_buffer;

internal void
win32_init_audio(HWND window) {
	HMODULE dsound_dll = LoadLibraryA("dsound.dll");
	if (!dsound_dll) {
		assert(0);
		return;
	}

	Direct_Sound_Create *direct_sound_create = (Direct_Sound_Create*)GetProcAddress(dsound_dll, "DirectSoundCreate");
	if (!direct_sound_create) {
		assert(0);
		return;
	}

	LPDIRECTSOUND direct_sound = 0;
	if (SUCCEEDED(direct_sound_create(0, &direct_sound, 0))) {

		if (SUCCEEDED(direct_sound->lpVtbl->SetCooperativeLevel(direct_sound, window, DSSCL_PRIORITY))) {
			
			// Sound spec

			sound_buffer.channel_count = 2;
			sound_buffer.samples_per_second = 44100;
			sound_buffer.bytes_per_sample = sound_buffer.channel_count*sizeof(s16);
			sound_buffer.size = sound_buffer.samples_per_second*sound_buffer.bytes_per_sample;

			DSBUFFERDESC buffer_description = {0};
			buffer_description.dwSize = sizeof(buffer_description);
			buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

			LPDIRECTSOUNDBUFFER primary_buffer;

			direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &primary_buffer, 0);

			WAVEFORMATEX wave_format = {0};
			wave_format.wFormatTag = WAVE_FORMAT_PCM;
			wave_format.nChannels = (WORD)sound_buffer.channel_count;
			wave_format.nSamplesPerSec = sound_buffer.samples_per_second;
			wave_format.wBitsPerSample = 16;
			wave_format.nBlockAlign = wave_format.nChannels * wave_format.wBitsPerSample/8;
			wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;


			buffer_description = (DSBUFFERDESC){0};
			buffer_description.dwSize = sizeof(buffer_description);
			buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS;
			buffer_description.dwBufferBytes = sound_buffer.size;
			buffer_description.lpwfxFormat = &wave_format;

			if (SUCCEEDED(direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &win32_sound_buffer, 0))); //Success
			else invalid_code_path;


		}
		else invalid_code_path;
	}
	else invalid_code_path;
}

internal void
win32_clear_sound_bufer() {

	void *region_1;
	DWORD region_1_size;
	void *region_2;
	DWORD region_2_size;

	if (SUCCEEDED(win32_sound_buffer->lpVtbl->Lock(win32_sound_buffer, 0, sound_buffer.size, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {

		s16 *at = region_1;

		DWORD region_1_sample_count = region_1_size/sound_buffer.bytes_per_sample;
		for (DWORD i = 0; i < region_1_sample_count; ++i) {

			*at++ = 0;
			*at++ = 0;
		}

		at = region_2;

		DWORD region_2_sample_count = region_2_size/sound_buffer.bytes_per_sample;
		for (DWORD i = 0; i < region_2_sample_count; ++i) {

			*at++ = 0;
			*at++ = 0;
		}

		if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Unlock(win32_sound_buffer, region_1, region_1_size, region_2, region_2_size))) invalid_code_path;
	}
	else invalid_code_path;
}

internal void
win32_fill_sound_bufer(DWORD byte_to_lock, DWORD bytes_to_write) {

	void *region_1;
	DWORD region_1_size;
	void *region_2;
	DWORD region_2_size;

	if (SUCCEEDED(win32_sound_buffer->lpVtbl->Lock(win32_sound_buffer, byte_to_lock, bytes_to_write, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {

		s16 *dest = region_1;
		s16 *source = sound_buffer.samples;

		DWORD region_1_sample_count = region_1_size/sound_buffer.bytes_per_sample;
		for (DWORD i = 0; i < region_1_sample_count; ++i) {

			*dest++ = *source++;
			*dest++ = *source++;

			sound_buffer.running_sample_index++;
		}

		dest = region_2;

		DWORD region_2_sample_count = region_2_size/sound_buffer.bytes_per_sample;
		for (DWORD i = 0; i < region_2_sample_count; ++i) {

			*dest++ = *source++;
			*dest++ = *source++;

			sound_buffer.running_sample_index++;
		}

		if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Unlock(win32_sound_buffer, region_1, region_1_size, region_2, region_2_size))) invalid_code_path;
	}
	else invalid_code_path;

}

OS_JOB_CALLBACK(win32_update_audio) {
	f32 target_dt = .01666f;

	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);

	for(;;) {
		DWORD safety_bytes = (int)((f32)sound_buffer.samples_per_second*(f32)sound_buffer.bytes_per_sample*target_dt); // One frame of safety
		safety_bytes -= safety_bytes % sound_buffer.bytes_per_sample;

		DWORD play_cursor, write_cursor;
		win32_sound_buffer->lpVtbl->GetCurrentPosition(win32_sound_buffer, &play_cursor, &write_cursor);
		
		DWORD byte_to_lock = (sound_buffer.running_sample_index*sound_buffer.bytes_per_sample) % sound_buffer.size;
		
		DWORD expected_bytes_per_tick = (DWORD)((f32)sound_buffer.samples_per_second*(f32)sound_buffer.bytes_per_sample*target_dt);
		expected_bytes_per_tick -= expected_bytes_per_tick % sound_buffer.bytes_per_sample;

		DWORD expected_boundary_byte = play_cursor + expected_bytes_per_tick;
		
		DWORD safe_write_buffer = write_cursor;
		if (safe_write_buffer < play_cursor) safe_write_buffer += sound_buffer.size;
		else safe_write_buffer += safety_bytes;

		DWORD target_cursor;
		if (safe_write_buffer < expected_boundary_byte) target_cursor = expected_boundary_byte + expected_bytes_per_tick;
		else target_cursor = write_cursor + expected_bytes_per_tick + safety_bytes;
		target_cursor %= sound_buffer.size;

		DWORD bytes_to_write;
		if (byte_to_lock > target_cursor) bytes_to_write = sound_buffer.size - byte_to_lock + target_cursor;
		else bytes_to_write = target_cursor - byte_to_lock;

		if (bytes_to_write) {
			sound_buffer.samples_to_write = bytes_to_write / sound_buffer.bytes_per_sample;
			assert(bytes_to_write % 4 == 0);
			update_audio(&sound_buffer, target_dt);
			win32_fill_sound_bufer(byte_to_lock, bytes_to_write);
		}

		f32 elapsed_time = os_seconds_elapsed(last_counter.QuadPart);
		QueryPerformanceCounter(&last_counter);
		int sleep = max(0, (int)(1000.f*(target_dt - elapsed_time))-1);
		Sleep(sleep);
	}
}

///////////////////////
// Callback and Main

internal LRESULT 
window_callback (HWND window, UINT message, WPARAM w_param, LPARAM l_param) {

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
			w32_bitmap_info.bmiHeader.biSize = sizeof(w32_bitmap_info.bmiHeader);
			w32_bitmap_info.bmiHeader.biWidth = render_buffer.width;
			w32_bitmap_info.bmiHeader.biHeight = render_buffer.height;
			w32_bitmap_info.bmiHeader.biPlanes = 1;
			w32_bitmap_info.bmiHeader.biBitCount = 32;
			w32_bitmap_info.bmiHeader.biCompression = BI_RGB;
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

	win32_init_audio(window);
	win32_clear_sound_bufer();
	if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Play(win32_sound_buffer, 0, 0, DSBPLAY_LOOPING))) invalid_code_path;

	sound_buffer.samples = VirtualAlloc(0, sound_buffer.size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	Os_Job_Queue audio_queue;
	win32_create_queue(&audio_queue, 1);
	os_add_job_to_queue(&audio_queue, win32_update_audio, (void *)0);

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
		if (!pause) update_game(&input, last_dt, &running);

		
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
					  &w32_bitmap_info,
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