#ifndef WIIUPCX_ZUI_H_
#define WIIUPCX_ZUI_H_
#include <stdint.h>
#include "utils.h"

enum zui_comp_type {
	ZUI_COMP_TYPE_TEXT
};

struct zui_point {
	int x, y;
};

struct zui_size {
	int w, h;
};



extern int zui_init(void* framebuffer, int w, int h);
extern void zui_term(void);

extern int zui_static_text_create(int winid, const char* str, struct vec2i origin);
extern void zui_update(void);
extern void zui_render(void);

#endif /* WIIUPCX_ZUI_H_ */


