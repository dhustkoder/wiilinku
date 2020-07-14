#include <coreinit/memdefaultheap.h>
#include <nsysnet/socket.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "udp.h"

static int udp_socket = -1;
static volatile int udp_lock = 0;


int udp_init(const char * ipString, unsigned short ipport)
{
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udp_socket < 0)
		return 0;

	struct sockaddr_in connect_addr;
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = ipport;
	inet_aton(ipString, &connect_addr.sin_addr);

	if(connect(udp_socket, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) < 0) {
		socketclose(udp_socket);
		udp_socket = -1;
		return 0;
	}

	return 1;
}

void udp_deinit(void)
{
	if (udp_socket >= 0) {
		socketclose(udp_socket);
		udp_socket = -1;
	}
}

int udp_send(uint8_t* data, int size)
{
	int len = size;
	while (len > 0) {
		int block = len < 1400 ? len : 1400; // take max 1400 bytes per UDP packet
		int ret = send(udp_socket, data, block, 0);
		if (ret < 0)
			return 1;
		len -= ret;
		data += ret;
	}

	return 0;
}

int udp_error(void)
{
	int error_code;
	socklen_t len = sizeof(error_code);
	getsockopt(udp_socket, SOL_SOCKET, SO_ERROR, &error_code, &len);
	return error_code;
}
