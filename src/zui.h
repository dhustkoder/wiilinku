#ifndef WIIUPCX_ZUI_H_
#define WIIUPCX_ZUI_H_
#include <stdint.h>
#include "utils.h"


extern bool zui_init(const char* title, void* framebuffer, int w, int h);
extern void zui_term(void);
extern int zui_static_text_create(int winid, const char* str, struct vec2i origin);
extern void zui_update(void);
extern void zui_render(void);

#endif /* WIIUPCX_ZUI_H_ */


