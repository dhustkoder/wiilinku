
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
#include "inputman.h"
#include "connection.h"


#define MAX_PACKET_BLOCK_SIZE (1400)


static int cmd_sock = 0;


static bool sock_wait_for_data(int sock) 
{
	int retval;
	fd_set readfd;

	struct timeval timer = {.tv_sec = 0, .tv_usec = 0};
	const unsigned long long ticks_to_wait = OSMillisecondsToTicks(60);
	const unsigned long long ticks = OSGetSystemTick();

	FD_ZERO(&readfd);
	FD_SET(sock, &readfd);

	do {
		retval = select(sock + 1, &readfd, NULL, NULL, &timer);
		if (retval != 0)
			break;
	} while ((OSGetSystemTick() - ticks) < ticks_to_wait);

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

static int cmd_packet_thread_main(int argc, const char** argv)
{
	struct cmd_packet cmd;
	memset(&cmd, 0, sizeof cmd);

	cmd.type = CMD_PACKET_TYPE_INPUT;

	for (;;) {
		inputman_update(&cmd.input);

		if (cmd.input.gamepad.btns&WIIU_GAMEPAD_BTN_HOME)
			goto Lexit;

		if (!send_packet(cmd_sock, &cmd, sizeof cmd))
			log_debug("send cmd packet failed");
	}
Lexit:
	return EXIT_SUCCESS;
}


bool connection_init(void)
{
	cmd_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (cmd_sock < 0)
		return false;

	return true;
}

void connection_term(void)
{
	socketclose(cmd_sock);
}


bool connection_connect(const char* ip, short port)
{
	struct sockaddr_in host;
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_port = port;
	inet_aton(ip, &host.sin_addr);

	if (connect(cmd_sock, (struct sockaddr*)&host, sizeof(host)) < 0) {
		log_debug("socket connect failed");
		return false;
	}

	OSRunThread(OSGetDefaultThread(0), cmd_packet_thread_main, 0, NULL);
	
	return true;
}


