#include <winsock2.h>
#include <ViGEm/Client.h>
#include <stdio.h>
#include <stdint.h>
#define ZED_NET_IMPLEMENTATION

#include "zed_net.h"




char recv_buffer[32];

VOID CALLBACK notification(
    PVIGEM_CLIENT Client,
    PVIGEM_TARGET Target,
    UCHAR LargeMotor,
    UCHAR SmallMotor,
    UCHAR LedNumber,
    LPVOID UserData
)
{
    printf("NOTIFICATION CALLED\n");
}


int main(int argc, char **argv) 
{

    PVIGEM_CLIENT client = vigem_alloc();

    VIGEM_ERROR ret = vigem_connect(client);

    PVIGEM_TARGET x360 = vigem_target_x360_alloc();

	ret = vigem_target_add(client, x360);

	ret = vigem_target_x360_register_notification(client, x360, &notification, NULL);

    XUSB_REPORT report;
    XUSB_REPORT_INIT(&report);
/*
    while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
        ret = vigem_target_x360_update(client, x360, report);
        report.bLeftTrigger++;
        Sleep(10);
    }
*/


    zed_net_init();

	const unsigned short port = 4242;
    zed_net_socket_t socket;
    if (zed_net_udp_socket_open(&socket, port, 0)) {
        printf("Error: %s\n", zed_net_get_error());
        return -1;
    }

    printf("waiting data on port: %d!\n", port);

    for (;;) {
        zed_net_address_t sender;
        int bytes_read = zed_net_udp_socket_receive(&socket, &sender, &recv_buffer, sizeof(recv_buffer));
        if (bytes_read) {
            uint16_t buttons;
            sscanf(recv_buffer, "%X", &buttons);
            report.wButtons = buttons;
            printf("RECV: %.4X\n", buttons);
        }
         ret = vigem_target_x360_update(client, x360, report);
    }


    vigem_target_x360_unregister_notification(x360);
    vigem_target_remove(client, x360);
    vigem_target_free(x360);
    vigem_free(client);

    zed_net_socket_close(&socket);
    zed_net_shutdown();
    return 0;
}
