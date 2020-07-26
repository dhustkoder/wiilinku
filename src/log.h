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

typedef void(*log_user_flusher_fp_t)(const char* buf, int size);


extern void log_term(void);
extern bool log_init(log_user_flusher_fp_t uflusher);
extern void log_flush(void);
extern void log_internal_write(enum log_level lvl, const char* fmt, ...);


#define log_info(...) log_internal_write(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_error(...) log_internal_write(LOG_LEVEL_ERROR, __VA_ARGS__)
#ifdef DEBUG
#define log_debug(...) log_internal_write(LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif

#endif /* WIIUPCX_LOG_H_ */


#ifdef LOG_IMPLEMENTATION

#ifndef LOG_INTERNAL_BUFFER_SIZE
#define LOG_INTERNAL_BUFFER_SIZE (4096)
#endif /* LOG_INTERNAL_BUFFER_SIZE */

#ifndef LOG_MAX_LINES
#define LOG_MAX_LINES 10
#endif



static log_user_flusher_fp_t log_user_flusher_fp;
static char log_internal_buffer[LOG_INTERNAL_BUFFER_SIZE];
static int log_internal_buffer_idx = 0;

void log_internal_write(enum log_level lvl, const char* fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);
	log_internal_buffer_idx += vsprintf(log_internal_buffer + log_internal_buffer_idx, fmt, valist);
	va_end(valist);

	log_internal_buffer[log_internal_buffer_idx++] = '\n';
	log_internal_buffer[log_internal_buffer_idx] = '\0';

	
	#ifdef LOG_IMMEDIATE_MODE
	
	log_user_flusher_fp(log_internal_buffer, log_internal_buffer_idx);
	log_internal_buffer_idx = 0;

	#else 

	const int nlines = str_chr_cnt(log_internal_buffer, '\n') - 1;
	if (nlines > LOG_MAX_LINES) {
		int lines_to_remove = nlines - LOG_MAX_LINES;
		for (int i = 0; i < lines_to_remove; ++i) {
			const int nw_idx = strchr(log_internal_buffer, '\n') - log_internal_buffer;
			const int movlen = strlen(log_internal_buffer + nw_idx + 1);
			memmove(log_internal_buffer, log_internal_buffer + nw_idx + 1, movlen);
			log_internal_buffer_idx -= nw_idx + 1;
			log_internal_buffer[log_internal_buffer_idx] = '\0';
		}
	}

	#endif
}


bool log_init(log_user_flusher_fp_t uflusher)
{
	log_user_flusher_fp = uflusher;
	memset(log_internal_buffer, 0, sizeof log_internal_buffer);
	return true;
}

void log_term(void)
{

}

void log_flush(void)
{
	log_user_flusher_fp(log_internal_buffer, log_internal_buffer_idx);
}


#endif /* LOG_IMPLEMENTATION */





