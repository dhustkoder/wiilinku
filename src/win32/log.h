#ifndef WIIUPCX_LOG_H_
#define WIIUPCX_LOG_H_
#include <windows.h>
#include <stdint.h>

enum stdhandle_index {
	WIIUPCX_LOG_STDOUT,
	WIIUPCX_LOG_STDERR
};


extern void log_term(void);
extern int log_init(void);
extern void log_internal_write(enum stdhandle_index idx, const char* fmt, ...);


#define log_info(fmtstr, ...) log_internal_write(WIIUPCX_LOG_STDOUT, fmtstr, __VA_ARGS__)
#define log_error(fmtstr, ...) log_internal_write(WIIUPCX_LOG_STDERR, fmtstr, __VA_ARGS__)
#ifdef DEBUG
#define log_debug(fmtstr, ...) log_internal_write(WIIUPCX_LOG_STDOUT, fmtstr, __VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif

#endif
