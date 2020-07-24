#ifndef WIIUPCX_UTILS_H_
#define WIIUPCX_UTILS_H_

struct vec2i {
	int x, y;
};

struct size2i {
	int w, h;
};

struct recti {
	int x, y;
	int w, h;
};

struct rgb24 {
	unsigned char r, g, b;
};

static int str_chr_cnt(const char* str, char c)
{
	int n = 0;
	for (; *str != '\0'; ++str) {
		if (*str == c)
			++n;
	}
	return n;
}

static int str_longest_line_len(const char* str)
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