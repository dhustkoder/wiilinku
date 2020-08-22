#ifndef WIILINKU_SOCKETS_H_
#define WIILINKU_SOCKETS_H_
#include "base_defs.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET socket_t;
typedef int socklen_t;

#define inet_aton(ip, saddr)        InetPton(AF_INET, ip, (saddr))
#define WLU_CLOSE_SOCKET(sock)      closesocket(sock)
#define WLU_SOCKET_GET_LAST_ERROR() WSAGetLastError()
#define WLU_SOCKET_WAIT_SEC 1
#define WLU_SOCKET_ERROR SOCKET_ERROR
#define WLU_INVALID_SOCKET INVALID_SOCKET

#elif defined(__WIIU__)

#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <nsysnet/socket.h>

typedef int socket_t;

#define WLU_SOCKET_WAIT_SEC         (0)
#define WLU_INVALID_SOCKET          (-1)
#define WLU_CLOSE_SOCKET(sock)      socketclose(sock)
#define WLU_SOCKET_GET_LAST_ERROR() (0)
#define WLU_SOCKET_ERROR SO_ERROR


#else
#error "Unknown Platform"
#endif



extern bool send_packet(socket_t sock, const void* data, int size);
extern bool recv_packet(socket_t sock, void* data, int size);

extern socket_t sockets_udp_send_create(const char* ip, u16 port);
extern socket_t sockets_udp_recv_create(u16 port);

#ifdef WLU_HOST
extern socket_t sockets_tcp_wait_client(u16 port, struct sockaddr_in* accepted_addr);
#else
extern socket_t sockets_tcp_connect_to_host(const char* ip, u16 port);
#endif

extern void sockets_close_socket(socket_t* sock);




#endif
