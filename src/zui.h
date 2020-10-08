#ifndef WIILINKU_ZUI_H_
#define WIILINKU_ZUI_H_
#include <stdint.h>
#include "utils.h"

#ifdef __WIIU__
#define ZUI_WIDTH  (426)
#define ZUI_HEIGHT (240)
#elif defined(_WIN32)
#define ZUI_WIDTH  (640)
#define ZUI_HEIGHT (360)
#endif

#define ZUI_OBJS_BUFFER_MAX_SIZE (512)
#define ZUI_MAX_OBJS             (16)
#define ZUI_MAX_COMMANDS         (16)

typedef int zui_obj_id_t;
typedef void(*zui_btn_clbk_fn_t)(void);

extern void zui_init(void);
extern void zui_term(void);
extern bool zui_update();
extern void zui_render(struct rgb24* dest);


extern zui_obj_id_t zui_text_create(struct vec2i origin);
extern void zui_text_set(const zui_obj_id_t id, const char* str);
extern void zui_text_draw(zui_obj_id_t id);
extern void zui_text_erase(zui_obj_id_t id);


extern zui_obj_id_t zui_btn_create(
	struct vec2i origin,
	const char* label,
	zui_btn_clbk_fn_t clbk
);
extern void zui_btn_draw(zui_obj_id_t id);





#endif /* WIILINKU_ZUI_H_ */


