#ifndef WIILINKU_HOST_GUI_H_
#define WIILINKU_HOST_GUI_H_
#include "utils.h"

enum gui_event {
	GUI_EVENT_NONE = 0,
	GUI_EVENT_WM_DESTROY = 1
};

typedef int gui_event_t;


extern bool gui_init(void);
extern void gui_term(void);
extern gui_event_t gui_update(void);
extern void gui_set_connection_status(bool connected, const char* clientaddr);
extern void gui_set_connection_local_ip(const char* ip);


#endif
