#include <stdint.h>
#include <string.h>
#include "sockets.h"



static bool sock_wait_for_data(socket_t sock, int sec, int usec) 
{
	fd_set readfd;

	FD_ZERO(&readfd);
	FD_SET(sock, &readfd);
	struct timeval timer = {
		.tv_sec = sec,
		.tv_usec = usec
	};

	int retval = select(0, &readfd, NULL, NULL, &timer);

	return retval > 0;
}

static bool setup_socket(
	socket_t* sock,
	int proto,
	struct sockaddr_in* addr,
	const char* ip,
	unsigned short port
)
{
	if (*sock != WLU_INVALID_SOCKET)
		sockets_close_socket(sock);

    *sock = socket(
    	AF_INET, 
    	proto == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM,
    	proto
    );

    if (*sock == WLU_INVALID_SOCKET) {
    	log_debug("socket() failed: %d", WLU_SOCKET_GET_LAST_ERROR());
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


static bool init_recv_socket(
	socket_t* sock,
	int proto,
	socket_t* client_sock,
	struct sockaddr_in* accepted_addr,
	short port
)
{
	struct sockaddr_in address;
	if (!setup_socket(sock, proto, &address, NULL, port))
		return false;

	if (bind(*sock, (const struct sockaddr*)&address, sizeof address) != 0) {
		log_debug("bind() failed: %d", WLU_SOCKET_GET_LAST_ERROR());
		return false;
	}

	if (proto == IPPROTO_TCP && client_sock != NULL && accepted_addr != NULL) {
		if (listen(*sock, 1) != 0) {
			log_debug("listen() failed: %d", WLU_SOCKET_GET_LAST_ERROR());
			return false;
		}

		if (*client_sock != WLU_INVALID_SOCKET) {
			sockets_close_socket(client_sock);
		}

		socklen_t addrlen = sizeof *accepted_addr;
		memset(accepted_addr, 0, addrlen);
		*client_sock = accept(*sock, (struct sockaddr*)accepted_addr, &addrlen);
		log_debug("connected to: %s", inet_ntoa(accepted_addr->sin_addr));
	}

	return true;
}

static bool init_send_socket(socket_t* sock, int proto, const char* ip, short port)
{
	log_debug("trying to init send socket to %s", ip);
	struct sockaddr_in host;
 	if (!setup_socket(sock, proto, &host, ip, port))
 		return false;

	if (connect(*sock, (struct sockaddr*)&host, sizeof(host)) != 0) {
		log_debug("connect() failed: %d", WLU_SOCKET_GET_LAST_ERROR());
		return false;
	}
	
	return true;
}



socket_t sockets_udp_send_create(const char* ip, short port)
{
	socket_t sock;
	
	if (!init_send_socket(&sock, IPPROTO_UDP, ip, port)) {
		sockets_close_socket(&sock);
		return WLU_INVALID_SOCKET;
	}

	return sock;
}

socket_t sockets_udp_recv_create(short port)
{
	socket_t sock;

	if (!init_recv_socket(&sock, IPPROTO_UDP, NULL, NULL, port)) {
		sockets_close_socket(&sock);
		return WLU_INVALID_SOCKET;
	}

	return sock;
}

socket_t sockets_tcp_wait_client(short port, struct sockaddr_in* accepted_addr)
{
	socket_t listener_sock;
	socket_t client_sock;
	const bool success = init_recv_socket(
		&listener_sock,
		IPPROTO_TCP,
    	&client_sock,
    	accepted_addr,
		port
    );

    sockets_close_socket(&listener_sock);

    if (!success) {
    	sockets_close_socket(&client_sock);
    	return WLU_INVALID_SOCKET;
    }

    return client_sock;
}

socket_t sockets_tcp_connect_to_host(const char* ip, short port)
{
	socket_t sock;
	if (!init_send_socket(&sock, IPPROTO_TCP, ip, port)) {
		sockets_close_socket(&sock);
		return WLU_INVALID_SOCKET;
	}

	return sock;
}

void sockets_close_socket(socket_t* sock)
{
	if (*sock != WLU_INVALID_SOCKET) {
		WLU_CLOSE_SOCKET(*sock);
		*sock = WLU_INVALID_SOCKET;
	}
}


bool send_packet(socket_t sock, const void* data, int size)
{
	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = send(
			sock,
			data,
			block_size, 
			0
		);

		if (ret < block_size || ret == WLU_SOCKET_ERROR) {
			log_debug("send_packet failed: %d", WLU_SOCKET_GET_LAST_ERROR());
			return false;
		}

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}

bool recv_packet(socket_t sock, void* data, int size)
{

	if (!sock_wait_for_data(sock, WLU_SOCKET_WAIT_SEC, 0))
		return false;

	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = recv(
			sock,
			data,
			block_size,
			0
		);

		if (ret < block_size || ret == WLU_SOCKET_ERROR) {
			log_debug("recv_packet failed: %d", WLU_SOCKET_GET_LAST_ERROR());
			return false;
		}

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}











