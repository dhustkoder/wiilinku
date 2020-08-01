
#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <nsysnet/socket.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "log.h"
#include "input.h"
#include "connection.h"


#define MAX_PACKET_BLOCK_SIZE (1400)


static int input_socket = 0;
static int input_feedback_socket = 0;


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

		if (ret < block_size)
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

		if (ret < block_size)
			return false;

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}

static bool init_recv_socket(int* sock, short port)
{
    *sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*sock <= 0) {
    	log_info("failed to start socket");
    	return false;
    }

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(*sock, (const struct sockaddr*)&address, sizeof address) != 0) {
		log_info("failed to bind socket");
		return false;
	}

	return true;
}

static bool init_send_socket(int* sock, const char* ip, short port)
{
	*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (*sock < 0)
		return false;

	struct sockaddr_in host;
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_port = htons(port);
	inet_aton(ip, &host.sin_addr);

	if (connect(*sock, (struct sockaddr*)&host, sizeof(host)) < 0) {
		log_debug("socket connect failed");
		return false;
	}	
	
	return true;
}

bool connection_init(void)
{
	if (!init_send_socket(&input_socket, "192.168.15.7", 7173))
		return false;
	if (!init_recv_socket(&input_feedback_socket, 7174))
		return false;

	return true;
}

void connection_term(void)
{
	socketclose(input_socket);
	socketclose(input_feedback_socket);
}

void connection_send_input_packet(const struct input_packet* input)
{
	send_packet(input_socket, input, sizeof *input);
}

bool connection_receive_input_feedback_packet(struct input_feedback_packet* feedback)
{
	if (recv_packet(input_feedback_socket, feedback, sizeof *feedback)) {
		return true;
	}

	return false;
}

