#ifndef WIILINKU_INPUT_H_
#define WIILINKU_INPUT_H_
#include <vpad/input.h>
#include "packets.h"



extern void input_init(void);
extern void input_term(void);
extern bool input_fetch(struct input_packet* input); /* thread-safe */
extern void input_fetch_vpad(VPADStatus* target);
extern bool input_update(struct input_packet* input);
extern void input_update_feedback(const struct input_feedback_packet* fb);


#endif /* WIILINKU_INPUTMAN_H_ */
