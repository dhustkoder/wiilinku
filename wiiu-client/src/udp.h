#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* A ripoff of logger.h */

int udp_init(const char * ip, unsigned short ipport);
void udp_deinit(void);
void udp_print(const char *str);
void udp_send(uint8_t* data, int size);
void udp_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif
