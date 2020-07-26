#ifndef WIIUPCX_HOST_GUI_H_
#define WIIUPCX_HOST_GUI_H_
#include "utils.h"

enum gui_event {
	GUI_EVENT_NONE = 0,
	GUI_EVENT_WM_DESTROY = 1
};

typedef int gui_event_t;


extern bool gui_init(
	const char* ip,
	unsigned short port
);

extern gui_event_t gui_win_update(void);

extern void gui_term(void);


#endif
