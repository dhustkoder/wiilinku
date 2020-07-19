#ifndef WIIUPCX_NETN_H_
#define WIIUPCX_NETN_H_

#if !defined(_WIN32) && !defined(__WIIU__)
#error "platform not supported"
#endif /* PLATFORM SUPPORT CHECK */


#ifdef _WIN32
#include "zed_net.h"
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
#endif /* __WIIU__ */


#include <stdint.h>


#define MAX_PACKET_LEN (1400)


enum netn_conn_idx {
	#ifdef WIIUPCX_HOST
	NETN_CONNECTION_JIN,
	NETN_CONNECTION_VOUT,
	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT
	NETN_CONNECTION_JOUT,
	NETN_CONNECTION_VIN,
	#endif /* WIIUPCX_CLIENT */

	NETN_CONNECTION_NCONS = 2,
};

struct netn_conn {
	#ifdef WIIUPCX_HOST
	zed_net_socket_t socket;
	zed_net_address_t client;
	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT
	int socket;
	struct sockaddr_in host;
	#endif /* WIIUPCX_CLIENT */
};

struct netn_joy_packet {
	#ifdef WIIUPCX_HOST
	uint32_t btns;
	int16_t lsx;
	int16_t lsy;
	int16_t rsx;
	int16_t rsy;
	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT
	uint32_t btns;
	int16_t lsx;
	int16_t lsy;
	int16_t rsx;
	int16_t rsy;
	#endif /* WIIUPCX_CLIENT */
};


extern int netn_init(void);
extern void netn_term(void);
extern int netn_send(const unsigned char* data, int size, enum netn_conn_idx idx);
extern int netn_recv(unsigned char* dest, int size, enum netn_conn_idx idx);
extern int netn_joy_update(struct netn_joy_packet* jpkt);
extern int netn_video_update(unsigned char* data, int w, int h, int bpp);

#endif /* WIIUPCX_NETN_H_ */

#ifdef NETN_IMPLEMENTATION /* NETN_IMPLEMENTATION */


#ifdef WIIUPCX_HOST
#define ZED_NET_IMPLEMENTATION
#include "zed_net.h"
#include "log.h"
#endif /* WIIUPCX_HOST */




static struct netn_conn conns[NETN_CONNECTION_NCONS];


#ifdef WIIUPCX_CLIENT

static int netn_client_conn_init(const char* ip, unsigned short port, struct netn_conn* conn)
{
	conn->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (conn->socket < 0)
		return 1;

	memset(&conn->host, 0, sizeof(conn->host));
	conn->host.sin_family = AF_INET;
	conn->host.sin_port = port;
	inet_aton(ip, &conn->host.sin_addr);

	if (connect(conn->socket, (struct sockaddr*)&conn->host, sizeof(conn->host)) < 0) {
		socketclose(conn->socket);
		conn->socket = -1;
		return 1;
	}

	return 0;
}

static void netn_client_conn_term(struct netn_conn* conn)
{
	socketclose(conn->socket);
	conn->socket = -1;
}

#endif /* WIIUPCX_CLIENT */


int netn_init(void)
{
	#ifdef WIIUPCX_HOST
	if (zed_net_init()) {
		log_info("error initializing zed net\n");
		return 1;
	}

	if (zed_net_udp_socket_open(&conns[NETN_CONNECTION_JIN].socket, 4242, 0)) {
		log_info("Error: %s\n", zed_net_get_error());
		return 1;
	}

	if (zed_net_udp_socket_open(&conns[NETN_CONNECTION_VOUT].socket, 4243, 0)) {
		log_info("Error: %s\n", zed_net_get_error());
		return 1;
	}

	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT

	

	if (netn_client_conn_init("192.168.15.7", 4242, &conns[NETN_CONNECTION_JOUT])) {
		return 1;
	}

	if (netn_client_conn_init("192.168.15.7", 4243, &conns[NETN_CONNECTION_VIN])) {
		return 1;
	}

	#endif /* WIIUPCX_CLIENT */

	return 0;
}

void netn_term(void)
{
	#ifdef WIIUPCX_HOST
	zed_net_socket_close(&conns[NETN_CONNECTION_JIN].socket);
	zed_net_socket_close(&conns[NETN_CONNECTION_VOUT].socket);
	zed_net_shutdown();
	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT
	netn_client_conn_term(&conns[NETN_CONNECTION_JOUT]);
	netn_client_conn_term(&conns[NETN_CONNECTION_VIN]);
	#endif /* WIIUPCX_CLIENT */
}

int netn_send(const unsigned char* data, int size, enum netn_conn_idx idx)
{
	struct netn_conn* conn = &conns[idx];

	while (size > 0) {
		const int bytes_sent = size > MAX_PACKET_LEN ? MAX_PACKET_LEN : size;

		#ifdef WIIUPCX_HOST
		int ret = zed_net_udp_socket_send(
			&conn->socket,
			conn->client,
			data,
			bytes_sent
		);
		#endif /* WIIUPCX_HOST */

		#ifdef WIIUPCX_CLIENT
		int ret = send(conn->socket, data, bytes_sent, 0);
		#endif /* WIIUPCX_CLIENT */

		if (ret < 0) {
			#ifdef WIIUPCX_HOST
			log_info("ERROR SENDING: %s\n", zed_net_get_error());
			#endif
			return -1;
		}

		size -= bytes_sent;
		data += bytes_sent;
	}

	return 0;
}

int netn_recv(unsigned char* dest, int size, enum netn_conn_idx idx)
{
	struct netn_conn* conn = &conns[idx];

	int len = size;
	while (len > 0) {
		const int bytes_to_recv = len > MAX_PACKET_LEN ? MAX_PACKET_LEN : len;

		#ifdef WIIUPCX_HOST
		int bytes_recv = zed_net_udp_socket_receive(
			&conn->socket,
			&conn->client,
			dest,
			bytes_to_recv
		);
		#endif /* WIIUPCX_HOST */

		#ifdef WIIUPCX_CLIENT
		int bytes_recv = recv(conn->socket, dest, bytes_to_recv, MSG_DONTWAIT);
		#endif

		if (bytes_recv <= 0) {
			#ifdef WIIUPCX_HOST
			log_info("ERROR RECEIVING: %s\n", zed_net_get_error());
			#endif /* WIIUPCX_HOST */
			return -1;
		}

		len -= bytes_recv;
		dest += bytes_recv;
	}

	return size;
}

int netn_joy_update(struct netn_joy_packet* jpkt)
{
	#ifdef WIIUPCX_HOST
	netn_recv((unsigned char*) jpkt, sizeof(*jpkt), NETN_CONNECTION_JIN);
	
	const uint32_t btns = jpkt->btns;
	const int16_t rsx = jpkt->rsx;
	const int16_t rsy = jpkt->rsy;
	const int16_t lsx = jpkt->lsx;
	const int16_t lsy = jpkt->lsy;

	jpkt->btns = (btns&0xFF)<<24|
	             (btns&0xFF000000)>>24|
	             (btns&0xFF0000)>>8|
	             (btns&0x00FF00)<<8;

	jpkt->rsx = (rsx&0xFF00)>>8|
	            (rsx&0x00FF)<<8;
	jpkt->rsy = (rsy&0xFF00)>>8|
	            (rsy&0x00FF)<<8;

	jpkt->lsx = (lsx&0xFF00)>>8|
	            (lsx&0x00FF)<<8;

	jpkt->lsy = (lsy&0xFF00)>>8|
	            (lsy&0x00FF)<<8;
	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT
	netn_send((unsigned char*) jpkt, sizeof(*jpkt), NETN_CONNECTION_JOUT);
	#endif /* WIIUPCX_CLIENT */

	return 0;
}

int netn_video_update(unsigned char* data, int w, int h, int bpp)
{
	#ifdef WIIUPCX_HOST
	netn_send(data, w * h * bpp, NETN_CONNECTION_VOUT);
	#endif /* WIIUPCX_HOST */

	#ifdef WIIUPCX_CLIENT
	netn_recv(data, w * h * bpp, NETN_CONNECTION_VIN);
	#endif /* WIIUPCX_CLIENT */

	return 0;
}

#endif /* NETN_IMPLEMENTATION */

