#ifndef WIIUPCX_CONNECTION_H_
#define WIIUPCX_CONNECTION_H_
#include <stdbool.h>
#include "packets.h"


typedef void(*input_packet_handler_fn_t)(
	const struct input_feedback_packet* feedback,
	struct input_packet* input
);


extern bool connection_init(input_packet_handler_fn_t input_packet_handler);
extern void connection_term(void);
extern bool connection_connect(const char* ip, short port);

#endif
