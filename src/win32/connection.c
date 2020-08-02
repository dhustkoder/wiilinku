#include <WinSock2.h>
#include <assert.h>
#include "log.h"
#include "connection.h"


#define MAX_PACKET_BLOCK_SIZE (1400)


static char hostname[80];
static struct in_addr local_network_ip;
static struct sockaddr_in client_addr;
static int client_addr_length = sizeof client_addr;


static SOCKET ping_pong_socket;
static SOCKET ping_pong_client_socket;
static SOCKET input_socket;
static SOCKET input_feedback_socket;
static bool connected = false;



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

static bool setup_socket(
	SOCKET* sock,
	int proto,
	struct sockaddr_in* addr,
	const char* ip,
	unsigned short port
)
{
	if (*sock > -1)
		closesocket(*sock);

    *sock = socket(
    	AF_INET, 
    	proto == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM,
    	proto
    );

    if (*sock < 0) {
    	log_info("failed to start socket");
    	return false;
    }

    memset(addr, 0, sizeof *addr);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (ip == NULL) {
		addr->sin_addr.s_addr = INADDR_ANY;
	} else {
		addr->sin_addr.s_addr = inet_addr(ip);
	}

	return true;
}

static bool init_recv_socket(
	SOCKET* sock,
	int proto,
	SOCKET* client_sock,
	struct sockaddr_in* client_addr,
	short port
)
{
	struct sockaddr_in address;
	setup_socket(sock, proto, &address, NULL, port);

	if (bind(*sock, (const struct sockaddr*)&address, sizeof address) != 0) {
		log_info("failed to bind socket");
		return false;
	}

	if (proto == IPPROTO_TCP) {
		if (listen(*sock, 1) != 0) {
			log_error("Couldnt listen server socket");
			return false;
		}

		log_info(
			"host %s listening on: %s:%d",
			hostname,
			inet_ntoa(local_network_ip),
			PING_PONG_PACKET_PORT
		);

		int addrlen = sizeof *client_addr;
		memset(client_addr, 0, addrlen);
		*client_sock = accept(*sock, (struct sockaddr*)client_addr, &addrlen);
		log_info("connected to: %s", connection_get_client_address());
	}

	return true;
}

 bool init_send_socket(SOCKET* sock, int proto, const char* ip, short port)
{
	struct sockaddr_in host;
 	setup_socket(sock, proto, &host, ip, port);

	if (connect(*sock, (struct sockaddr*)&host, sizeof(host)) < 0) {
		log_debug("socket connect failed");
		return false;
	}	
	
	return true;
}



static bool fill_local_host_and_port(void)
{
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

	return true;
}

static DWORD WINAPI connection_wait_thread_main(LPVOID p)
{
    if (!init_recv_socket(
    		&ping_pong_socket,
    	    IPPROTO_TCP,
    	    &ping_pong_client_socket,
    	    &client_addr,
    	    PING_PONG_PACKET_PORT)
    	) {
    	return false;
	}

	if (!init_recv_socket(&input_socket, IPPROTO_UDP, NULL, NULL, INPUT_PACKET_PORT))
		return false;

	if (!init_send_socket(
			&input_feedback_socket,
			IPPROTO_UDP,
			inet_ntoa(client_addr.sin_addr),
			INPUT_FEEDBACK_PACKET_PORT)
		) {
		return false;
	}

	connected = true;

	return true;
}


bool connection_init(void)
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_info("WSAStartup Error");
        return false;
    }

	if (!fill_local_host_and_port()) 
		return false;

	if (!connection_wait_thread_main(NULL))
		return false;

	return true;
}

void connection_term(void)
{
	closesocket(input_socket);
	closesocket(input_feedback_socket);
	closesocket(ping_pong_socket);
	closesocket(ping_pong_client_socket);
	WSACleanup();
}

const char* connection_get_host_address(void)
{
	return inet_ntoa(local_network_ip);
}

const char* connection_get_client_address(void)
{
	return inet_ntoa(client_addr.sin_addr);
}

bool connection_is_connected(void)
{
	return connected;
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
