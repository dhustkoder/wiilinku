#include <WinSock2.h>
#include <iphlpapi.h>
#include <assert.h>
#include "log.h"
#include "connection.h"


static char local_ip_buf[24] = "000.000.000.000\0";
static struct sockaddr_in client_addr;


static SOCKET ping_tcp_socket = INVALID_SOCKET;
static SOCKET ping_client_socket = INVALID_SOCKET;
static SOCKET input_socket = INVALID_SOCKET;
static SOCKET input_feedback_socket = INVALID_SOCKET;
static bool connected = false;




static bool sock_wait_for_data(SOCKET sock, int sec, int usec) 
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

static bool send_packet(SOCKET sock, const void* data, int size)
{
	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = send(
			sock,
			data,
			block_size, 
			0
		);

		if (ret < block_size || ret == SOCKET_ERROR) {
			log_debug("send_packet failed: %d", WSAGetLastError());
			return false;
		}

		size -= block_size;
		data = ((uint8_t*)data) + block_size;
	}

	return true;
}

static bool recv_packet(SOCKET sock, void* data, int size)
{

	if (!sock_wait_for_data(sock, 1, 0))
		return false;

	while (size > 0) {
		const int block_size = size > MAX_PACKET_BLOCK_SIZE ? MAX_PACKET_BLOCK_SIZE : size;

		int ret = recv(
			sock,
			data,
			block_size,
			0
		);

		if (ret < block_size || ret == SOCKET_ERROR) {
			log_debug("recv_packet failed: %d", WSAGetLastError());
			return false;
		}

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
	if (*sock != INVALID_SOCKET)
		closesocket(*sock);

    *sock = socket(
    	AF_INET, 
    	proto == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM,
    	proto
    );

    if (*sock < 0) {
    	log_debug("failed to start socket");
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
	struct sockaddr_in* accepted_addr,
	short port
)
{
	struct sockaddr_in address;
	setup_socket(sock, proto, &address, NULL, port);

	if (bind(*sock, (const struct sockaddr*)&address, sizeof address) != 0) {
		log_debug("failed to bind socket");
		return false;
	}

	if (proto == IPPROTO_TCP) {
		if (listen(*sock, 1) != 0) {
			log_error("Couldnt listen server socket");
			return false;
		}

		if (*client_sock != INVALID_SOCKET) {
			closesocket(*client_sock);
		}

		int addrlen = sizeof *accepted_addr;
		memset(accepted_addr, 0, addrlen);
		*client_sock = accept(*sock, (struct sockaddr*)accepted_addr, &addrlen);
		log_debug("connected to: %s", inet_ntoa(client_addr.sin_addr));
	}

	return true;
}

static bool init_send_socket(SOCKET* sock, int proto, const char* ip, short port)
{
	struct sockaddr_in host;
 	setup_socket(sock, proto, &host, ip, port);

	if (connect(*sock, (struct sockaddr*)&host, sizeof(host)) < 0) {
		log_debug("socket connect failed");
		return false;
	}	
	
	return true;
}

static bool setup_local_ip_buf(void)
{
	PIP_ADAPTER_INFO ip_info_buffer = malloc(sizeof(*ip_info_buffer));
	PIP_ADAPTER_INFO itr = NULL;
	PIP_ADAPTER_INFO found = NULL;
	ULONG size = sizeof *ip_info_buffer;
	ULONG err;
	err = GetAdaptersInfo(ip_info_buffer, &size);

	if (err != NO_ERROR) {
		ip_info_buffer = realloc(ip_info_buffer, size);
		err = GetAdaptersInfo(ip_info_buffer, &size);
		if (err != NO_ERROR) {
			free(ip_info_buffer);
			return false;
		}
	}

	log_debug(" --- Searching For Local Network Ip Address ---");
	itr = ip_info_buffer;
	while (itr != NULL) {
		log_debug("---------------------------------------------");
		log_debug("name: %s", itr->AdapterName);
		log_debug("desc: %s", itr->Description);
		log_debug("type: %u", itr->Type);
		log_debug("addr: %s", itr->IpAddressList.IpAddress.String);
		log_debug("gateway: %s", itr->GatewayList.IpAddress);
		log_debug("---------------------------------------------");
		if (itr->Type == IF_TYPE_IEEE80211 || itr->Type == MIB_IF_TYPE_ETHERNET) {
			if (strcmp(itr->IpAddressList.IpAddress.String, "0.0.0.0") != 0) {
				found = itr;
				break;
			}
		}
		itr = itr->Next;
	}
	log_debug("--- Search Ended ---");

	if (found == NULL) {
		log_debug("local ip address / adapter not found");
		free(ip_info_buffer);
		return false;
	}

	/* fill in the ip string buffer backwards to keep left 0s */
	const size_t local_ip_buf_len = strlen(local_ip_buf);
	const size_t src_ip_buf_len = strlen(found->IpAddressList.IpAddress.String);
	char* dest_itr = local_ip_buf + local_ip_buf_len;
	const char* src_itr = found->IpAddressList.IpAddress.String + src_ip_buf_len;
	while (dest_itr > &local_ip_buf[0]) {
		--src_itr;
		--dest_itr;

		if (*src_itr == '.') {
			while (*dest_itr != '.')
				--dest_itr;
			--src_itr;
			--dest_itr;
		}

		*dest_itr = *src_itr;
	}

	log_debug("local_ip_buf result: %s", local_ip_buf);
	
	free(ip_info_buffer);
	return true;
}

bool connection_wait_client(void)
{
	bool success = init_recv_socket(
		&ping_tcp_socket,
		IPPROTO_TCP,
    	&ping_client_socket,
    	&client_addr,
		PING_PACKET_PORT
    );

	if (!success)
		return false;

	success = init_recv_socket(
		&input_socket,
		IPPROTO_UDP,
		NULL,
		NULL,
		INPUT_PACKET_PORT
	);

	if (!success)
		return false;

	success = init_send_socket(
		&input_feedback_socket,
		IPPROTO_UDP,
		inet_ntoa(client_addr.sin_addr),
		INPUT_FEEDBACK_PACKET_PORT
	);

	if (!success)
		return false;

	connected = true;
	return true;
}

bool connection_ping_client(void)
{
	if (connection_is_connected()) {
		uint8_t ping = 0xFF;
		if (send_packet(ping_client_socket, &ping, sizeof ping)) {
			return true;
		} else {
			log_debug("disconnected from: %s", inet_ntoa(client_addr.sin_addr));
			connected = false;
			return false;
		}
	}

	return false;
}


bool connection_init(void)
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        log_debug("WSAStartup Error");
        return false;
    }

	if (!setup_local_ip_buf()) 
		return false;

	return true;
}

void connection_term(void)
{
	closesocket(input_socket);
	closesocket(input_feedback_socket);
	closesocket(ping_tcp_socket);
	closesocket(ping_client_socket);

	ping_tcp_socket = INVALID_SOCKET;
	ping_client_socket = INVALID_SOCKET;
	input_socket = INVALID_SOCKET;
	input_feedback_socket = INVALID_SOCKET;

	WSACleanup();
}

const char* connection_get_host_address(void)
{
	return local_ip_buf;
}

const char* connection_get_client_address(void)
{
	return inet_ntoa(client_addr.sin_addr);
}

bool connection_is_connected(void)
{
	return connected;
}

bool connection_recv_input_packet(struct input_packet* input)
{
	if (connected) {
		if (recv_packet(input_socket, input, sizeof *input)) {
			input_packet_reorder(input);
			return true;
		}
	}

	return false;
}

void connection_send_input_feedback_packet(const struct input_feedback_packet* feedback)
{
	if (connected) {
		send_packet(input_feedback_socket, feedback, sizeof *feedback);
	}
}
