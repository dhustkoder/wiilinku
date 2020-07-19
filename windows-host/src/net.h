#ifndef WIIUPCX_NET_H_
#define WIIUPCX_NET_H_
#include <winsock2.h>
#include <windows.h>

enum net_connection {
	NET_CONNECTION_JIN,
	NET_CONNECTION_VOUT,
};

int net_init(void);
void net_term(void);
int net_send_packet(const BYTE* data, size_t size, enum net_connection conn);
int net_recv_packet(BYTE* dest, size_t size, enum net_connection conn);

#endif