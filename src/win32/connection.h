#ifndef WIIUPCX_CONNECTION_H_
#define WIIUPCX_CONNECTION_H_
#include "packets.h"


extern bool connection_init(void);
extern void connection_term(void);
extern void connection_get_address(char** ip, short* port);

#endif
