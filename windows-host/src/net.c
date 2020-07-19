#define ZED_NET_IMPLEMENTATION
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "zed_net.h"



#include "log.h"
#include "net.h"



#define MAX_BLOCK_LEN (1400)

static struct connection {
	zed_net_socket_t socket;
	zed_net_address_t client;
} conns[2];


int net_init(void)
{
	if (zed_net_init()) {
		log_info("error initializing zed net\n");
		return 1;
	}

	if (zed_net_udp_socket_open(&conns[NET_CONNECTION_JIN].socket, 4242, 1)) {
		log_info("Error: %s\n", zed_net_get_error());
		return 1;
	}

	if (zed_net_udp_socket_open(&conns[NET_CONNECTION_VOUT].socket, 4243, 1)) {
		log_info("Error: %s\n", zed_net_get_error());
		return 1;
	}

	#define WIIUPCX_DEBUG
	#ifdef WIIUPCX_DEBUG
	zed_net_get_address(&conns[NET_CONNECTION_VOUT].client, "localhost", 4243);
	#endif

	log_info("waiting data on port: %d!", 4242);
	return 0;
}

void net_term(void)
{
	zed_net_socket_close(&conns[NET_CONNECTION_JIN].socket);
	zed_net_shutdown();
}

int net_send_packet(const BYTE* data, int size, enum net_connection conidx)
{
	struct connection* conn = &conns[conidx];

	while (size > 0) {
		const int bytes_sent = size > MAX_BLOCK_LEN ? MAX_BLOCK_LEN : size;

		int ret = zed_net_udp_socket_send(
			&conn->socket,
			conn->client,
			data,
			bytes_sent
		);

		if (ret == -1) {
			log_info("ERROR SENDING: %s\n", zed_net_get_error());
			return -1;
		}

		size -= bytes_sent;
		data += bytes_sent;
	}

	return 0;
}

int net_recv_packet(BYTE* dest, int size, enum net_connection conidx)
{
	struct connection* conn = &conns[conidx];

	int len = size;
	while (len > 0) {
		const int bytes_to_recv = len > MAX_BLOCK_LEN ? MAX_BLOCK_LEN : len;

		int bytes_recv = zed_net_udp_socket_receive(
			&conn->socket,
			&conn->client,
			dest,
			bytes_to_recv
		);

		if (bytes_recv == -1 || bytes_recv == 0) {
			log_info("ERROR RECEIVING: %s\n", zed_net_get_error());
			return -1;
		}

		len -= bytes_recv;
		dest += bytes_recv;
	}

	return size;
}
