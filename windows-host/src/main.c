#include <winsock2.h>
#include <stdio.h>
#define ZED_NET_IMPLEMENTATION

#include "zed_net.h"

int main(int argc, char **argv) {
   zed_net_init();

	const unsigned short port = 4242;
    zed_net_socket_t socket;
    if (zed_net_udp_socket_open(&socket, port, 0)) {
        printf("Error: %s\n", zed_net_get_error());
        return -1;
    }

    printf("waiting data on port: %d!\n", port);

    char buffer[256];
	

    while (strcmp(buffer, "done") != 0) {
        zed_net_address_t sender;
		memset(buffer, 0, sizeof(buffer));
        int bytes_read = zed_net_udp_socket_receive(&socket, &sender, &buffer, sizeof(buffer));
        if (bytes_read) {
            printf(
				"Received %d bytes from %s:%d: %s\n", 
				bytes_read, 
				zed_net_host_to_str(sender.host), 
				sender.port, 
				buffer
			);
            printf("Echoing...\n");
            zed_net_udp_socket_send(&socket, sender, buffer, sizeof(buffer));
        }
    }

    printf("Done!\n");
    zed_net_socket_close(&socket);
    zed_net_shutdown();
    return 0;
}
