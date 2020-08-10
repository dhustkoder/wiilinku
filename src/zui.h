#ifndef WIILINKU_ZUI_H_
#define WIILINKU_ZUI_H_
#include <stdint.h>
#include "utils.h"


extern void zui_init(const char* title, void* framebuffer, int w, int h);
extern void zui_term(void);
extern int zui_static_text_create(int winid, const char* str, struct vec2i origin);
extern bool zui_update(void);
extern void zui_render(void);
extern int zui_dynamic_text_create(
	int winid,
	int max_line_len,
	int max_lines
);
extern void zui_dynamic_text_set(int obj_id, const char* str, struct vec2i origin);




#endif /* WIILINKU_ZUI_H_ */


