#ifndef WIILINKU_ZUI_H_
#define WIILINKU_ZUI_H_
#include <stdint.h>
#include "utils.h"

#define ZUI_WIDTH  (720)
#define ZUI_HEIGHT (480)

typedef int zui_obj_id_t;

extern void zui_init(void);
extern void zui_term(void);
extern zui_obj_id_t zui_text_create(
	int max_line_length,
	int max_lines,
	struct vec2i coord
);
extern void zui_text_set(zui_obj_id_t id, const char* str);
extern bool zui_update(struct rgb24* dest);




#endif /* WIILINKU_ZUI_H_ */


