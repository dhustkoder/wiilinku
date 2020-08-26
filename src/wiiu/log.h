#ifndef WIILINKU_LOG_H_
#define WIILINKU_LOG_H_
#include <whb/log.h>
#include <whb/log_udp.h>
#include "base_defs.h"


#ifdef WLU_DEBUG

#define log_init()     WHBLogUdpInit()
#define log_term()     WHBLogUdpDeinit()
#define log_info(...)  WHBLogPrintf(__VA_ARGS__)
#define log_error(...) WHBLogPrintf(__VA_ARGS__)
#define log_debug(...) WHBLogPrintf(__VA_ARGS__)

#else

#define log_init() (true)
#define log_term() ((void)0)
#define log_info(...) ((void)0) 
#define log_error(...) ((void)0)
#define log_debug(...) ((void)0)


#endif


#endif