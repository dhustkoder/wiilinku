#ifndef WIIUPCX_INPUT_H_
#define WIIUPCX_INPUT_H_
#include "packets.h"


extern void input_init(void);
extern void input_term(void);
extern void input_fetch(struct input_packet* input);


#endif /* WIIUPCX_INPUTMAN_H_ */
