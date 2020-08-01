#ifndef WIIUPCX_INPUTMAN_H_
#define WIIUPCX_INPUTMAN_H_
#include "packets.h"


extern void inputman_init(void);
extern void inputman_term(void);

extern void inputman_update(
	const struct input_feedback_packet* feedback,
	struct input_packet* input
);

extern void inputman_fetch(struct input_packet* input);


#endif /* WIIUPCX_INPUTMAN_H_ */
