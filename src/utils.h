#ifndef WIILINKU_UTILS_H_
#define WIILINKU_UTILS_H_
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>


#ifndef WLU_VERSION_STR
#error "Version str missing"
#endif


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;




#define WLU_MAX_ALIGNMENT_SIZE sizeof(uintptr_t)


/* debug / assert */
#ifdef WLU_DEBUG

#ifdef _WIN32
#include "log.h"


#define WLU_ASSERT(cond) {                                     \
	if (!(cond)) {                                             \
		log_error("ASSERT FAILED %s:%d", __FILE__, __LINE__);  \
		__debugbreak();                                        \
	}                                                          \
}


#elif defined(__WIIU__)

#include <coreinit/thread.h>
#include "log.h"

#define WLU_ASSERT(cond) {                                           \
	if (!(cond)) {                                                   \
		log_info("ASSERT FAILED %s:%d", __FILE__, __LINE__);         \
		OSSleepTicks(OSSecondsToTicks(10));                          \
	}                                                                \
}


#elif defined (__APPLE__)

#include <signal.h>
#include "log.h"

#define WLU_ASSERT(cond) {                                   \
	if (!(cond)) {                                           \
		log_error("ASSERT FAILED %s:%d", __FILE__, __LINE__);\
		__builtin_trap();                                    \
	}                                                        \
}

#else
#error "Unkown Platform"
#endif /* _WIN32 / __WIIU__ */

#else

#define WLU_ASSERT(...) ((void)0)

#endif /* DEBUG */


/* compiler utils */
#ifdef _WIN32
#define WLU_UNUSED(x) ((void)x)
#elif __GNUC__
#define WLU_UNUSED(x) ((void)x)
#endif /* MSVC / GCC */


#ifdef _WIN32
#define BSWAP_16(x) _byteswap_ushort(x)
#define BSWAP_32(x) _byteswap_ulong(x)
#define BSWAP_64(x) _byteswap_uint64(x)


#elif __GNUC__
#define BSWAP_16(x) __builtin_bswap16(x)
#define BSWAP_32(x) __builtin_bswap32(x)
#define BSWAP_64(x) __builtin_bswap64(x)

#else
#error "Unknown Platform"
#endif /* MSVC / GCC */


#define FMT_STR_VARGS_EX(targetbuf, maxsize, written, fmt, lastarg) {     \
	va_list utils__va_list;                                               \
	va_start(utils__va_list, lastarg);                                    \
	written = vsprintf(targetbuf, fmt, utils__va_list);                   \
	WLU_ASSERT(written < maxsize);                                        \
	va_end(utils__va_list);                                               \
}

#define FMT_STR_VARGS(targetbuf, maxsize, fmt, lastarg) {               \
	unsigned utils__written;                                            \
	WLU_UNUSED(utils__written);                                         \
	FMT_STR_VARGS_EX(targetbuf, maxsize, utils__written, fmt, lastarg); \
}


struct vec2i {
	int x, y;
};

struct recti {
	struct vec2i coord;
	struct vec2i size;
};

struct rgb24 {
	unsigned char r, g, b;
};



inline static int str_chr_cnt(const char* str, char c)
{
	int n = 0;
	for (; *str != '\0'; ++str) {
		if (*str == c)
			++n;
	}
	return n;
}

inline static int str_longest_line_len(const char* str)
{
	int longest_line_len = 0;
	int cnt = 0;
	for (;*str != '\0'; ++str) {
		if (*str == '\n') {
			
			if (cnt > longest_line_len)
				longest_line_len = cnt;
			
			cnt = 0;
			continue;
		}
		++cnt;
	}
	
	return longest_line_len > cnt ? longest_line_len : cnt;
}

inline static int str_line_len(const char* str)
{
	int len = 0;
	
	while (*str != '\0' && *str != '\n') {
		++str;
		++len;
	}

	return len;
}

inline static int str_cnt_lines(const char* str) 
{
	int nlines = 1;
	while (*str != '\0') {
		if (*str == '\n')
			++nlines;
		++str;
	}
	return nlines;
}

inline static const char* str_next_line(const char* str)
{
	while (*str != '\n' && *str != '\0')
		++str;

	while (*str == '\n')
		++str;

	return str;
}



#endif