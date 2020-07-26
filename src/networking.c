#ifdef _WIN32
#include <WinSock2.h>

typedef SOCKET socket_t;

#endif /* _WIN32 */


#ifdef __WIIU__
#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <nsysnet/socket.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

typedef int socket_t;

#endif /* __WIIU__ */



#include <assert.h>
#include "log.h"
#include "networking.h"


#define NETWORKING_MAX_RETRIES (6)
#define MAX_PACKET_BLOCK_SIZE (1400)


static socket_t joypad_sock = 0;

#ifdef WIIUPCX_HOST
static char hostname[80];
static struct in_addr local_network_ip;
static unsigned short local_network_port;
static struct sockaddr_in client_addr;
#endif /* WIIUPCX_HOST */



static bool sock_wait_for_data(socket_t sock) 
{
	fd_set readfd;

	FD_ZERO(&readfd);
	FD_SET(sock, &readfd);
	struct timeval timer;

#if defined(_WIN32)
	timer.tv_sec = 0;
	timer.tv_usec = 600000;

	int retval = select(sock + 1, &readfd, NULL, NULL, &timer);

#elif defined(__WIIU__)
	int retval;
	const unsigned long long ticks_to_wait = OSMillisecondsToTicks(60);
	const unsigned long long ticks = OSGetSystemTick();
	timer.tv_sec = 0;
	timer.tv_usec = 0;

	do {
		retval = select(sock + 1, &readfd, NULL, NULL, &timer);
		if (retval != 0)
			break;
	} while ((OSGetSystemTick() - ticks) < ticks_to_wait);

#endif

	/*
	static unsigned retval_cnt = 0;
	log_info("RETVAL: %d | %d", retval, retval_cnt++);
	*/

	return retval > 0;
}


static bool send_packet(socket_t sock, const void* data, int size)
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

static bool recv_packet(socket_t sock, void* data, int size)
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



#ifdef WIIUPCX_HOST


static bool host_recv_first_packet(socket_t sock, void* data, int size)
{
	if (!sock_wait_for_data(sock))
		return false;

	memset(&client_addr, 0, sizeof client_addr);

	int from_length = sizeof client_addr;

	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = recvfrom(
			sock,
			data,
			block_size,
			0,
			(struct sockaddr *) &client_addr,
			&from_length
		);

		if (ret < block_size)
			return false;

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}


static bool host_init_socks(void)
{
    joypad_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (joypad_sock <= 0) {
    	log_info("failed to start socket");
    	return false;
    }

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(7173);

	if (bind(joypad_sock, (const struct sockaddr*)&address, sizeof address) != 0) {
		log_info("failed to bind socket");
		return false;
	}

	return true;
}

static bool host_fill_ip_and_port(void)
{
	struct sockaddr_in address;
	int namelen = sizeof address;
	while (getsockname(joypad_sock, (struct sockaddr*)&address, &namelen) != 0) {
		log_info("failed to getsockname: %d", WSAGetLastError());
		return false;
	}

	if (gethostname(hostname, sizeof(hostname)) != 0) {
		log_info("failed to gethostname: %d", WSAGetLastError());
		return false;
	}

	struct hostent *phe = gethostbyname(hostname);
	if (phe == NULL || phe->h_addr_list == NULL || phe->h_addr_list[0] == NULL) {
		log_info("failed to gethostbyname");
		return false;
	}

	memcpy(&local_network_ip, phe->h_addr_list[0], sizeof(struct in_addr));
	local_network_port = ntohs(address.sin_port);

	return true;
}

bool networking_init(const char** ip, unsigned short* port)
{

#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_info("WSAStartup Error");
        return false;
    }
#endif /* _WIN32 */

    if (!host_init_socks())
    	return false;

	if (!host_fill_ip_and_port()) 
		return false;

	*ip = inet_ntoa(local_network_ip);
	*port = local_network_port;

	log_info(
		"host %s listening on: %s:%d",
		hostname,
		inet_ntoa(local_network_ip),
		(int)local_network_port
	);

	return true;
}


#endif /* WIIUPCX_HOST */





#ifdef WIIUPCX_CLIENT



bool networking_init(void)
{
	joypad_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (joypad_sock < 0)
		return false;

	return true;
}



#endif


void networking_term(void)
{
#ifdef _WIN32
	closesocket(joypad_sock);
	WSACleanup();
#endif /* _WIN32 */

#ifdef __WIIU__
	socketclose(joypad_sock);
#endif
}

#ifdef WIIUPCX_HOST
bool networking_try_handshake(void)
#else
bool networking_try_handshake(const char* ip, unsigned short port)
#endif
{
	struct command_packet cmd;


#ifdef WIIUPCX_HOST
	
	memset(&cmd, 0, sizeof cmd);
	

	if (!host_recv_first_packet(joypad_sock, &cmd, sizeof cmd)) {
		log_info("failed to recv handshake");
		closesocket(joypad_sock);
		host_init_socks();
		return false;
	}

	if (cmd.cmd != COMMAND_PACKET_HANDSHAKE) {
		log_info("recv wrong handshake packet");
		return false;
	}

	if (connect(joypad_sock, (struct sockaddr*) &client_addr, sizeof client_addr) < 0) {
		log_info("failed to connect to client");
		return false;
	}

	if (!send_packet(joypad_sock, &cmd, sizeof cmd)) {
		log_info("failed to send handshake");
		return false;
	}

#endif /* WIIUPCX_HOST */

#if WIIUPCX_CLIENT

	struct sockaddr_in host;
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_port = port;
	inet_aton(ip, &host.sin_addr);

	if (connect(joypad_sock, (struct sockaddr*)&host, sizeof(host)) < 0) {
		log_info("failed to connect");
		return false;
	}
	
	cmd.cmd = COMMAND_PACKET_HANDSHAKE;
	
	if (!send_packet(joypad_sock, &cmd, sizeof cmd)) {
		log_info("failed to send handshake");
		return false;
	}

	memset(&cmd, 0, sizeof cmd);

	if (!recv_packet(joypad_sock, &cmd, sizeof cmd)) {
		log_info("failed to recv handshake");
		return false;
	}

	if (cmd.cmd != COMMAND_PACKET_HANDSHAKE) {
		log_info("recv wrong handshake packet");
		return false;
	}

#endif /* WIIUPCX_CLIENT */



	return true;
}

bool networking_packet_exchange(socket_t sock, void* outpack, int outsize, void* inpack, int insize)
{
#ifdef WIIUPCX_HOST
	if (!recv_packet(sock, inpack, insize))
		return false;
	if (!send_packet(sock, outpack, outsize))
		return false;
#else
	if (!send_packet(sock, outpack, outsize))
		return false;
	if (!recv_packet(sock, inpack, insize))
		return false;
#endif

	return true;
}


bool networking_input_update(struct input_packet* packet)
{
	static struct command_packet cmd;
	int retries = 0;

#if defined(WIIUPCX_HOST)
	
	cmd.cmd = COMMAND_PACKET_RECEIVED;

	do {
		if (networking_packet_exchange(joypad_sock, &cmd, sizeof cmd, packet, sizeof *packet))
			break;
	} while (++retries < NETWORKING_MAX_RETRIES);

	if (retries >= NETWORKING_MAX_RETRIES)
		return false;

	uint32_t btns = packet->gamepad.btns;
	const int16_t rsx = packet->gamepad.rsx;
	const int16_t rsy = packet->gamepad.rsy;
	const int16_t lsx = packet->gamepad.lsx;
	const int16_t lsy = packet->gamepad.lsy;

	packet->gamepad.btns = (btns&0xFF)<<24|
	                     (btns&0xFF000000)>>24|
	                     (btns&0xFF0000)>>8|
	                     (btns&0x00FF00)<<8;

	packet->gamepad.rsx = (rsx&0xFF00)>>8|
	                    (rsx&0x00FF)<<8;
	packet->gamepad.rsy = (rsy&0xFF00)>>8|
	                    (rsy&0x00FF)<<8;

	packet->gamepad.lsx = (lsx&0xFF00)>>8|
	                    (lsx&0x00FF)<<8;

	packet->gamepad.lsy = (lsy&0xFF00)>>8|
	                    (lsy&0x00FF)<<8;

	btns = packet->wiimotes[0].btns;
	packet->wiimotes[0].btns = (btns&0xFF)<<24|
	                     (btns&0xFF000000)>>24|
	                     (btns&0xFF0000)>>8|
	                     (btns&0x00FF00)<<8;
   
#elif defined(WIIUPCX_CLIENT)
	memset(&cmd, 0, sizeof cmd);

	do {
		if (networking_packet_exchange(joypad_sock, packet, sizeof *packet, &cmd, sizeof cmd))
			break;
	} while (++retries < NETWORKING_MAX_RETRIES);

	if (retries >= NETWORKING_MAX_RETRIES || cmd.cmd != COMMAND_PACKET_RECEIVED)
		return false;

#endif

	return true;
}
