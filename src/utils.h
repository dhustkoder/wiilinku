#ifndef WIIUPCX_UTILS_H_
#define WIIUPCX_UTILS_H_
#include <stdint.h>
#include <stdbool.h>


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

#endif

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