#ifndef WIIUPCX_NETWORKING_H_
#define WIIUPCX_NETWORKING_H_
#include "input_packet.h"


#if !defined(_WIN32) && !defined(__WIIU__)
#error "unknown platform"
#endif



enum command_packet_commands {
	COMMAND_PACKET_RECEIVED  = 0x01,
	COMMAND_PACKET_HANDSHAKE = 0x1f
};

struct command_packet {
	uint8_t cmd;
};




#ifdef _WIN32
extern bool networking_init(const char** out_ip, unsigned short* out_port);
#elif defined(__WIIU__)
extern bool networking_init(void);
#endif
extern void networking_term(void);

#ifdef WIIUPCX_HOST
extern bool networking_try_handshake(void);
#endif /* WIIUPCX_HOST */

#ifdef WIIUPCX_CLIENT
extern bool networking_try_handshake(const char* ip, unsigned short port);
#endif /* WIIUPCX_CLIENT */

extern bool networking_input_update(struct input_packet* packet);




#endif
