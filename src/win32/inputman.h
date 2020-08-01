#ifndef WIIUPCX_INPUTMAN_H_
#define WIIUPCX_INPUTMAN_H_
#include <stdint.h>
#include "packets.h"


bool inputman_init(void);
void inputman_term(void);

void inputman_update(
	const struct input_packet* input,
	struct input_feedback_packet* feedback
);


#endif
