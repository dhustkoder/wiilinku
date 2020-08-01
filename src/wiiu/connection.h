#ifndef WIIUPCX_CONNECTION_H_
#define WIIUPCX_CONNECTION_H_
#include <stdbool.h>
#include "packets.h"

extern bool connection_init(void);
extern void connection_term(void);

extern void connection_send_input_packet(const struct input_packet* input);
extern bool connection_receive_input_feedback_packet(struct input_feedback_packet* feedback);


#endif
