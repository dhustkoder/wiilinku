#ifndef WIIUPCX_INPUTMAN_H_
#define WIIUPCX_INPUTMAN_H_
#include "input_packet.h"


extern void inputman_init(void);
extern void inputman_term(void);
extern void inputman_update(struct input_packet* pack);
extern void inputman_fetch(struct input_packet* pack);



#endif /* WIIUPCX_INPUTMAN_H_ */
