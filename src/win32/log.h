#ifndef WIIUPCX_LOG_H_
#define WIIUPCX_LOG_H_
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

enum log_level {
	LOG_LEVEL_INFO,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_NLEVELS
};


extern void log_term(void);
extern bool log_init(void);
extern void log_internal_write(enum log_level lvl, const char* fmt, ...);


#define log_info(...) log_internal_write(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_error(...) log_internal_write(LOG_LEVEL_ERROR, __VA_ARGS__)
#ifdef DEBUG
#define log_debug(...) log_internal_write(LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif

#endif /* WIIUPCX_LOG_H_ */








