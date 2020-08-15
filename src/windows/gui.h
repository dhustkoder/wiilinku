#ifndef WIILINKU_HOST_GUI_H_
#define WIILINKU_HOST_GUI_H_
#include "packets.h"
#include "utils.h"

enum gui_event {
	GUI_EVENT_NONE = 0,
	GUI_EVENT_WM_DESTROY = 1
};

typedef int gui_event_t;


extern bool gui_init(void);
extern void gui_term(void);
extern gui_event_t gui_update(void);
extern void gui_set_local_ip_string(const char* ip);
extern void gui_set_client_ip_string(const char* client_ip);
extern void gui_set_connected_controllers(input_packet_flags_t flags);


#endif
