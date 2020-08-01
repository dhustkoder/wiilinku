#include <WinSock2.h>
#include <assert.h>
#include "log.h"
#include "connection.h"


#define MAX_PACKET_BLOCK_SIZE (1400)


static char hostname[80];
static struct in_addr local_network_ip;
static unsigned short local_network_port;
static struct sockaddr_in client_addr;
static int client_addr_length = sizeof client_addr;


static SOCKET input_socket;
static SOCKET input_feedback_socket;




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

		int ret = send(
			sock,
			data,
			block_size, 
			0);

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

		int ret = recv(
			sock,
			data,
			block_size,
			0);

		if (ret < block_size)
			return false;

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}


static bool init_recv_socket(SOCKET* sock, short port)
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

 bool init_send_socket(SOCKET* sock, const char* ip, short port)
{
	*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (*sock < 0)
		return false;

	struct sockaddr_in host;
	memset(&host, 0, sizeof(host));
	host.sin_family = AF_INET;
	host.sin_port = htons(port);
	host.sin_addr.s_addr = inet_addr(ip);

	if (connect(*sock, (struct sockaddr*)&host, sizeof(host)) < 0) {
		log_debug("socket connect failed");
		return false;
	}	
	
	return true;
}


static bool fill_local_host_and_port(void)
{
	struct sockaddr_in address;
	int namelen = sizeof address;
	while (getsockname(input_socket, (struct sockaddr*)&address, &namelen) != 0) {
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




bool connection_init(void)
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_info("WSAStartup Error");
        return false;
    }

    memset(&client_addr, 0, sizeof client_addr);

	if (!init_recv_socket(&input_socket, 7173))
		return false;

	if (!init_send_socket(&input_feedback_socket, "192.168.15.4", 7174))
		return false;

	if (!fill_local_host_and_port()) 
		return false;

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
	closesocket(input_socket);
	closesocket(input_feedback_socket);
	WSACleanup();
}

void connection_get_address(char** ip, short* port)
{
	*ip = inet_ntoa(local_network_ip);
	*port = local_network_port;
}

bool connection_receive_input_packet(struct input_packet* input)
{
	if (recv_packet(input_socket, input, sizeof *input)) {
		input_packet_reorder(input);
		return true;
	}

	return false;
}

void connection_send_input_feedback_packet(const struct input_feedback_packet* feedback)
{
	send_packet(input_feedback_socket, feedback, sizeof *feedback);
}
