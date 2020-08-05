#ifndef WIIUPCX_INPUT_H_
#define WIIUPCX_INPUT_H_
#include <stdint.h>
#include "packets.h"


extern bool input_init(void);
extern void input_term(void);
extern void input_update(const struct input_packet* input);


#endif

