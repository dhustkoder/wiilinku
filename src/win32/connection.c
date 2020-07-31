#include <WinSock2.h>
#include <assert.h>
#include "x360emu.h"
#include "log.h"
#include "connection.h"


#define MAX_PACKET_BLOCK_SIZE (1400)


static char hostname[80];
static struct in_addr local_network_ip;
static unsigned short local_network_port;
static struct sockaddr_in client_addr;
static int client_addr_length = sizeof client_addr;
static SOCKET cmd_sock;
static HANDLE cmd_thread_handle;


static bool sock_wait_for_data(SOCKET sock) 
{
	fd_set readfd;

	FD_ZERO(&readfd);
	FD_SET(sock, &readfd);
	struct timeval timer = {
		.tv_sec = 1,
		.tv_usec = 0
	};

	int retval = select(0, &readfd, NULL, NULL, &timer);

	return retval > 0;
}


static bool send_packet(SOCKET sock, const void* data, int size)
{
	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = sendto(
			sock,
			data,
			block_size, 
			0,
			(struct sockaddr *) &client_addr,
			sizeof client_addr
		);

		if (ret < block_size)
			return false;

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}

static bool recv_packet(SOCKET sock, void* data, int size)
{

	if (!sock_wait_for_data(sock))
		return false;

	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = recvfrom(
			sock,
			data,
			block_size,
			0,
			(struct sockaddr *) &client_addr,
			&client_addr_length
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
    cmd_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (cmd_sock <= 0) {
    	log_info("failed to start socket");
    	return false;
    }

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(7173);

	if (bind(cmd_sock, (const struct sockaddr*)&address, sizeof address) != 0) {
		log_info("failed to bind socket");
		return false;
	}

	return true;
}

static bool host_fill_ip_and_port(void)
{
	struct sockaddr_in address;
	int namelen = sizeof address;
	while (getsockname(cmd_sock, (struct sockaddr*)&address, &namelen) != 0) {
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


static void input_packet_reorder(struct input_packet* p)
{
	uint32_t btns = p->gamepad.btns;
	const int16_t rsx = p->gamepad.rsx;
	const int16_t rsy = p->gamepad.rsy;
	const int16_t lsx = p->gamepad.lsx;
	const int16_t lsy = p->gamepad.lsy;

	p->gamepad.btns = BSWAP_32(btns);

	p->gamepad.rsx = BSWAP_16(rsx);

	p->gamepad.rsy = BSWAP_16(rsy);

	p->gamepad.lsx = BSWAP_16(lsx);

	p->gamepad.lsy = BSWAP_16(lsy);

	btns = p->wiimotes[0].btns;

	p->wiimotes[0].btns = BSWAP_32(btns);
}

static void input_packet_handler(struct input_packet* p)
{
	input_packet_reorder(p);
	x360emu_update(p);
}


static DWORD WINAPI cmd_packet_thread_main(LPVOID param)
{
	((void)param);

	DWORD avg_frame_ms;
	DWORD lasttick;
	DWORD tickacc = 0;
	int framecnt = 0;

	struct cmd_packet cmd_packet;


	for (;;) {
		lasttick = GetTickCount();

		while (!recv_packet(cmd_sock, &cmd_packet, sizeof cmd_packet)) {
			log_debug("failed to recv cmd packet");
		}

		if (cmd_packet.type == CMD_PACKET_TYPE_INPUT) {
			input_packet_reorder(&cmd_packet.input);
			x360emu_update(&cmd_packet.input);
		}

		tickacc += GetTickCount() - lasttick;
		if (++framecnt >= 60) {
			avg_frame_ms = tickacc / framecnt;
			tickacc = 0;
			framecnt = 0;
			log_debug("CMD PACKET AVERAGE FRAME TIME: %ld ms", avg_frame_ms);
		}
	}

	return EXIT_SUCCESS;
}


bool connection_init(void)
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_info("WSAStartup Error");
        return false;
    }

    memset(&client_addr, 0, sizeof client_addr);

    if (!host_init_socks())
    	return false;

	if (!host_fill_ip_and_port()) 
		return false;


	DWORD thread_id;
	cmd_thread_handle = CreateThread(
		NULL,
		0,
		cmd_packet_thread_main,
		NULL,
		0,
		&thread_id
	);

	log_info(
		"host %s listening on: %s:%d",
		hostname,
		inet_ntoa(local_network_ip),
		(int)local_network_port
	);

	return true;
}

void connection_term(void)
{
	if (!TerminateThread(cmd_thread_handle, 0)) {
		log_info("failed to terminate thread: %d", GetLastError());
	}

	closesocket(cmd_sock);
	WSACleanup();
}


void connection_get_address(char** ip, short* port)
{
	*ip = inet_ntoa(local_network_ip);
	*port = local_network_port;
}

