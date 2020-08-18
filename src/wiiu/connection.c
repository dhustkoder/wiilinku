
#include "video.h"
#include "input.h"
#include "connection.h"
#include "sockets.h"


static socket_t ping_socket = WLU_INVALID_SOCKET;
static socket_t input_socket = WLU_INVALID_SOCKET;
static socket_t input_feedback_socket = WLU_INVALID_SOCKET;
static bool connected = false;



static void close_all_sockets(void)
{
	sockets_close_socket(&input_feedback_socket);
	sockets_close_socket(&input_socket);
	sockets_close_socket(&ping_socket);
}


bool connection_init(void)
{
	socket_lib_init();
	return true;
}

void connection_term(void)
{
	connected = false;
	close_all_sockets();
	socket_lib_finish();
}

bool connection_connect(const char* host_ip)
{
	ping_socket = sockets_tcp_connect_to_host(host_ip, PING_PACKET_PORT);
	if (ping_socket == WLU_INVALID_SOCKET)
		goto Lfailed;

	input_socket = sockets_udp_send_create(host_ip, INPUT_PACKET_PORT);
	if (input_socket == WLU_INVALID_SOCKET)
		goto Lfailed;

	input_feedback_socket = sockets_udp_recv_create(INPUT_FEEDBACK_PACKET_PORT);
	if (input_feedback_socket == WLU_INVALID_SOCKET)
		goto Lfailed;

	connected = true;
	return true;
	
Lfailed:
	close_all_sockets();
	return false;
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

	if (recv_packet(input_feedback_socket, feedback, sizeof *feedback))
		return true;

	return false;
}


bool connection_ping_host(void)
{
	if (connection_is_connected()) {
		uint8_t ping = 0xFF;
		if (send_packet(ping_socket, &ping, sizeof ping)) {
			return true;
		} else {
			close_all_sockets();
			connected = false;
			return false;
		}
	}

	return false;
}