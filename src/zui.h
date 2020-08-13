#ifndef WIILINKU_ZUI_H_
#define WIILINKU_ZUI_H_
#include <stdint.h>
#include "utils.h"

#define ZUI_WIDTH  (720)
#define ZUI_HEIGHT (480)

#define ZUI_OBJS_BUFFER_MAX_SIZE (512)
#define ZUI_MAX_OBJS             (16)
#define ZUI_MAX_COMMANDS         (16)

typedef int zui_obj_id_t;

extern void zui_init(void);
extern void zui_term(void);
extern zui_obj_id_t zui_text_create(
	const char* str,
	struct vec2i coord
);
extern void zui_text_set(zui_obj_id_t id, const char* str);
extern bool zui_update();
extern void zui_render(struct rgb24* dest);




#endif /* WIILINKU_ZUI_H_ */


