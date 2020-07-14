#ifndef WIIUPCX_UDP_H_
#define WIIUPCX_UDP_H_
int udp_init(const char * ip, unsigned short ipport);
void udp_deinit(void);
int udp_send(uint8_t* data, int size);
int udp_error(void);
#endif
