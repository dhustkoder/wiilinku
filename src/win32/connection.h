#ifndef WIIUPCX_CONNECTION_H_
#define WIIUPCX_CONNECTION_H_
#include "packets.h"


typedef void(*input_packet_handler_fn_t)(
	const struct input_packet* input,
	struct input_feedback_packet* feedback
);


extern bool connection_init(
	input_packet_handler_fn_t input_packet_handler
);

extern void connection_term(void);
extern void connection_get_address(char** ip, short* port);


#endif
