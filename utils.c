#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;

typedef int16_t s16;
typedef uint16_t u16;

typedef int32_t s32;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef int b32;

#define true 1
#define false 0

#define global_variable static
#define internal static

global_variable b32 running = true;

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

#if DEVELOPMENT
#define assert(c) {if (!c) {*(int*)0=0;} else{}}
#else
#define assert(c)
#endif

#define invalid_default_case default : {assert(0);}
#define invalid_code_path assert(0);

inline void
zero_size(void *mem, u64 size) {
	u8 *dest = (u8*)mem;
	for (u64 i = 0; i < size; ++i) {
		*dest++ = 0;
	}
}

#define zero_array(a) zero_size(a, sizeof(a))

#define zero_struct(s) zero_size(&(s), sizeof(s))