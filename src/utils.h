#ifndef WIILINKU_UTILS_H_
#define WIILINKU_UTILS_H_
#include "base_defs.h"


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