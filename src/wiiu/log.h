#ifndef WIILINKU_LOG_H_
#define WIILINKU_LOG_H_
#include <whb/log.h>
#include <whb/log_udp.h>


#define log_info(...)  WHBLogPrintf(__VA_ARGS__)
#define log_error(...) WHBLogPrintf(__VA_ARGS__)
#ifdef WLU_DEBUG
#define log_debug(...) WHBLogPrintf(__VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif




#endif