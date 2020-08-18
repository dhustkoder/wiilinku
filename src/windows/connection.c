#include <winsock2.h>
#include <iphlpapi.h>
#include <assert.h>
#include "sockets.h"
#include "log.h"
#include "connection.h"
#include "error.h"


static char local_ip_buf[24] = "000.000.000.000\0";
static struct sockaddr_in client_addr;


static SOCKET ping_socket = INVALID_SOCKET;
static SOCKET input_socket = INVALID_SOCKET;
static SOCKET input_feedback_socket = INVALID_SOCKET;
static bool connected = false;


static bool setup_local_ip_buf(void)
{
	bool ret = false;
	ULONG err;
	PIP_ADAPTER_INFO ip_info_buffer = malloc(sizeof(*ip_info_buffer));
	PIP_ADAPTER_INFO itr = NULL;
	PIP_ADAPTER_INFO found = NULL;
	ULONG size = sizeof *ip_info_buffer;
	
	err = GetAdaptersInfo(ip_info_buffer, &size);

	if (err != NO_ERROR) {
		ip_info_buffer = realloc(ip_info_buffer, size);
		err = GetAdaptersInfo(ip_info_buffer, &size);
		if (err != NO_ERROR) {
			set_last_error("GetAdaptersInfo failed");
			goto Lret;
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
		log_debug("---------------------------------------------");
		if (itr->Type == IF_TYPE_IEEE80211 || itr->Type == MIB_IF_TYPE_ETHERNET) {
			if (strcmp(itr->IpAddressList.IpAddress.String, "0.0.0.0") != 0) {
				found = itr;
				break;
			}
		}
		itr = itr->Next;
	}
	log_debug("-------------- Search Ended --------------------");

	if (found == NULL) {
		set_last_error("local ip address or adapter not found");
		goto Lret;
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
	
	ret = true;
Lret:
	free(ip_info_buffer);
	return ret;
}

bool connection_wait_client(void)
{
    ping_socket = sockets_tcp_wait_client(PING_PACKET_PORT, &client_addr);
    if (ping_socket == WLU_INVALID_SOCKET)
    	goto Lfailed;

	input_socket = sockets_udp_recv_create(INPUT_PACKET_PORT);
	if (input_socket == WLU_INVALID_SOCKET)
		goto Lfailed;

	input_feedback_socket = sockets_udp_send_create(
		inet_ntoa(client_addr.sin_addr),
		INPUT_FEEDBACK_PACKET_PORT
	);
	if (input_feedback_socket == WLU_INVALID_SOCKET)
		goto Lfailed;

	connected = true;
	return true;

Lfailed:
	sockets_close_socket(&ping_socket);
	sockets_close_socket(&input_socket);
	sockets_close_socket(&input_feedback_socket);
	return false;
}

bool connection_ping_client(void)
{
	if (connection_is_connected()) {
		uint8_t ping = 0xFF;
		if (send_packet(ping_socket, &ping, sizeof ping)) {
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
        set_last_error("WSAStartup Error");
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
	closesocket(ping_socket);

	ping_socket = INVALID_SOCKET;
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
