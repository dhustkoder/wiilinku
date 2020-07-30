#ifndef WIIUPCX_X360EMU_H_
#define WIIUPCX_X360EMU_H_
#include <stdint.h>
#include "packets.h"





bool x360emu_init(void);
void x360emu_term(void);
void x360emu_update(struct input_packet* pack);

#endif