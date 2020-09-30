#ifndef WIILINKU_WLUDEFS_H_
#define WIILINKU_WLUDEFS_H_
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
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef intptr_t sptr;
typedef uintptr_t uptr;


#define WLU_MAX_ALIGNMENT_SIZE sizeof(uptr)


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
		log_error("ASSERT FAILED %s:%d", __FILE__, __LINE__);        \
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


#define WLU_FMT_STR_VARGS_EX(targetbuf, maxsize, written, fmt, lastarg) {     \
	va_list utils__va_list;                                                   \
	va_start(utils__va_list, lastarg);                                        \
	written = vsprintf(targetbuf, fmt, utils__va_list);                       \
	WLU_ASSERT(written < maxsize);                                            \
	va_end(utils__va_list);                                                   \
}

#define WLU_FMT_STR_VARGS(targetbuf, maxsize, fmt, lastarg) {               \
	unsigned utils__written;                                                \
	WLU_UNUSED(utils__written);                                             \
	FMT_STR_VARGS_EX(targetbuf, maxsize, utils__written, fmt, lastarg);     \
}



#endif