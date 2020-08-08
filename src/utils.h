#ifndef WIILINKU_UTILS_H_
#define WIILINKU_UTILS_H_
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

/* debug / assert */
#ifdef WIILINKU_DEBUG

#ifdef _WIN32
#include "win32/log.h"

#define WLU_ASSERT(cond) {                                     \
	if (!(cond)) {                                             \
		log_error("ASSERT FAILED %s:%s", __FILE__, __LINE__);  \
		__debugbreak();                                        \
	}                                                          \
}


#elif defined(__WIIU__)

#include <coreinit/thread.h>
#include "wiiu/video.h"

#define WLU_ASSERT(cond) {                                           \
	if (!(cond)) {                                                   \
		video_log_printf("ASSERT FAILED %s:%s", __FILE__, __LINE__); \
		OSSleepTicks(OSSecondsToTicks(10));                          \
	}                                                                \
}

#endif /* _WIN32 / __WIIU__ */

#else

#define WLU_ASSERT(...) ((void)0)

#endif /* DEBUG */


/* compiler utils */
#ifdef _WIN32
#define WLU_UNUSED(x) ((void)x)
#elif defined(__WIIU__)
#define WLU_UNUSED(x) ((void)x)
#endif


#ifdef _WIN32
#define BSWAP_16(x) _byteswap_ushort(x)
#define BSWAP_32(x) _byteswap_ulong(x)
#define BSWAP_64(x) _byteswap_uint64(x)


#elif defined(__WIIU__)
#define BSWAP_16(x) __builtin_bswap16(x)
#define BSWAP_32(x) __builtin_bswap32(x)
#define BSWAP_64(x) __builtin_bswap64(x)

#else
#define BSWAP_16(x) ((((x)&0xFF00)>>8)|(((x)&0x00FF)<<8))


#define BSWAP_32(x) (((((x)&0xFF000000)>>24)|(((x)&0x000000FF)<<24))| \
                    ((((x)&0x00FF0000)>>8)|(((x)&0x0000FF00)<<8)))


#define BSWAP_64(x) (((((x)&0xFF00000000000000)>>56)|(((x)&0x00000000000000FF)<<56))| \
                     ((((x)&0x00FF000000000000)>>40)|(((x)&0x000000000000FF00)<<40))| \
                     ((((x)&0x0000FF0000000000)>>24)|(((x)&0x0000000000FF0000)<<24))| \
                     ((((x)&0x000000FF00000000)>>8) |(((x)&0x00000000FF000000)<<8)))

#endif /* _WIN32 / __WIIU__ */


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






#endif