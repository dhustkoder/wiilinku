#ifndef WIIUPCX_CONNECTION_H_
#define WIIUPCX_CONNECTION_H_
#include <stdbool.h>
#include "packets.h"


extern bool connection_init(void);
extern void connection_term(void);
extern bool connection_connect(const char* ip, short port);

#endif
