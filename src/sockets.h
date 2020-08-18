#ifndef WIILINKU_SOCKETS_H_
#define WIILINKU_SOCKETS_H_


#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include "log.h"

typedef SOCKET socket_t;
typedef int socklen_t;

#define WLU_CLOSE_SOCKET(sock)      closesocket(sock)
#define WLU_SOCKET_GET_LAST_ERROR() WSAGetLastError()
#define WLU_SOCKET_WAIT_SEC 1
#define WLU_SOCKET_ERROR SOCKET_ERROR
#define WLU_INVALID_SOCKET INVALID_SOCKET
#define inet_aton(...) InetPtonW(AF_INET, __VA_ARGS__)


#elif defined(__WIIU__)

#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <nsysnet/socket.h>
#include "video.h"

typedef int socket_t;

#define WLU_SOCKET_WAIT_SEC 0
#define WLU_INVALID_SOCKET (-1)
#define WLU_CLOSE_SOCKET(sock) socketclose(sock)
#define WLU_SOCKET_GET_LAST_ERROR() (0)
#define WLU_SOCKET_ERROR SO_ERROR
#define log_debug(...) video_log_printf(__VA_ARGS__)


#else
#error "Unknown Platform"
#endif

#include <stdbool.h>


#define MAX_PACKET_BLOCK_SIZE (1400)


extern bool send_packet(socket_t sock, const void* data, int size);
extern bool recv_packet(socket_t sock, void* data, int size);

extern socket_t sockets_udp_send_create(const char* ip, short port);
extern socket_t sockets_udp_recv_create(short port);
extern socket_t sockets_tcp_wait_client(short port, struct sockaddr_in* accepted_addr);
extern socket_t sockets_tcp_connect_to_host(const char* ip, short port);

extern void sockets_close_socket(socket_t* socket);




#endif
