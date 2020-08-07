#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <nsysnet/socket.h>
#include <nn/ac/ac_c.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "video.h"
#include "input.h"
#include "connection.h"


static int ping_socket = -1;
static int input_socket = -1;
static int input_feedback_socket = -1;
static bool connected = false;


static bool sock_wait_for_data(int sock) 
{
	fd_set readfd;

	FD_ZERO(&readfd);
	FD_SET(sock, &readfd);
	struct timeval timer = {
		.tv_sec = 0,
		.tv_usec = 0
	};

	int retval = select(sock + 1, &readfd, NULL, NULL, &timer);

	return retval > 0;
}



static bool send_packet(int sock, const void* data, int size)
{
	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = send(sock, data, block_size, 0);

		if (ret < block_size || ret == SO_ERROR)
			return false;

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}

static bool recv_packet(int sock, void* data, int size)
{
	if (!sock_wait_for_data(sock))
		return false;

	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = recv(sock, data, block_size, 0);

		if (ret < block_size || ret == SO_ERROR)
			return false;

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}


static bool setup_socket(
	int* sock,
	int proto,
	struct sockaddr_in* addr,
	const char* ip,
	unsigned short port
)
{
	if (*sock > -1)
		socketclose(*sock);

    *sock = socket(
    	AF_INET, 
    	proto == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM,
    	proto
    );

    if (*sock < 0) {
    	video_log_printf("failed to start socket");
    	return false;
    }

    memset(addr, 0, sizeof *addr);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (ip == NULL) {
		addr->sin_addr.s_addr = INADDR_ANY;
	} else {
		inet_aton(ip, &addr->sin_addr);
	}

	return true;
}


static bool setup_recv_socket(int* sock, int proto, unsigned short port)
{
	struct sockaddr_in addr;
	if (!setup_socket(sock, proto, &addr, NULL, port))
		return false;

	if (bind(*sock, (const struct sockaddr*)&addr, sizeof addr) != 0) {
		video_log_printf("failed to bind socket");
		return false;
	}

	return true;
}

static bool setup_send_socket(int* sock, int proto, const char* ip, unsigned short port)
{
	struct sockaddr_in addr;
	if (!setup_socket(sock, proto, &addr, ip, port))
		return false;

	if (connect(*sock, (struct sockaddr*)&addr, sizeof addr) != 0) {
		video_log_printf("socket connect failed");
		return false;
	}	
	
	return true;
}


bool connection_init(void)
{
	socket_lib_init();
	return true;
}

void connection_term(void)
{
	connected = false;

	socketclose(input_feedback_socket);
	socketclose(input_socket);
	socketclose(ping_socket);
	socket_lib_finish();

	ping_socket = -1;
	input_socket = -1;
	input_feedback_socket = -1;

}

bool connection_connect(const char* host_ip)
{
	if (!setup_send_socket(&ping_socket, IPPROTO_TCP, host_ip, PING_PACKET_PORT))
		return false;

	if (!setup_send_socket(&input_socket, IPPROTO_UDP, host_ip, INPUT_PACKET_PORT))
		return false;

	if (!setup_recv_socket(&input_feedback_socket, IPPROTO_UDP, INPUT_FEEDBACK_PACKET_PORT))
		return false;

	connected = true;
	return true;
}

bool connection_is_connected(void)
{
	return connected;
}

void connection_send_input_packet(const struct input_packet* input)
{
	if (connected)
		send_packet(input_socket, input, sizeof *input);
}

bool connection_receive_input_feedback_packet(struct input_feedback_packet* feedback)
{
	if (!connected)
		return false;

	if (recv_packet(input_feedback_socket, feedback, sizeof *feedback)) {
		return true;
	}

	return false;
}


bool connection_ping_host(void)
{
	if (connection_is_connected()) {
		uint8_t ping = 0xFF;
		if (send_packet(ping_socket, &ping, sizeof ping)) {
			return true;
		} else {
			connected = false;
			return false;
		}
	}

	return false;
}