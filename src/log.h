#ifndef WIIUPCX_LOG_H_
#define WIIUPCX_LOG_H_

enum log_level {
	LOG_LEVEL_INFO,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_NLEVELS
};

typedef void(*log_user_flusher_fp_t)(const char* buf);


extern void log_term(void);
extern int log_init(log_user_flusher_fp_t uflusher);
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

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE (4096)
#endif /* LOG_BUFFER_SIZE */

#ifndef LOG_MAX_LINES
#define LOG_MAX_LINES 10
#endif



static log_user_flusher_fp_t log_user_flusher_fp;
static char log_buffer[LOG_BUFFER_SIZE];
static int log_buffer_idx = 0;


static int log_get_nlines(void)
{
	const char* p = log_buffer;
	int nlines = 0;

	while (*p != '\0')  {
		if (*p == '\n' && (*(p+1)) != '\0')
			++nlines;
		++p;
	}

	return nlines;
}


void log_internal_write(enum log_level lvl, const char* fmt, ...)
{
	va_list valist;
	va_start(valist, fmt);
	log_buffer_idx += vsprintf(log_buffer + log_buffer_idx, fmt, valist);
	va_end(valist);

	log_buffer[log_buffer_idx++] = '\n';
	log_buffer[log_buffer_idx] = '\0';

	
	#ifdef LOG_IMMEDIATE_MODE
	
	log_user_flusher_fp(log_buffer);
	log_buffer_idx = 0;

	#else 

	const int nlines = log_get_nlines();
	if (nlines > LOG_MAX_LINES) {
		int lines_to_remove = nlines - LOG_MAX_LINES;
		for (int i = 0; i < lines_to_remove; ++i) {
			const int nw_idx = strchr(log_buffer, '\n') - log_buffer;
			const int movlen = strlen(log_buffer + nw_idx + 1);
			memmove(log_buffer, log_buffer + nw_idx + 1, movlen);
			log_buffer_idx -= nw_idx + 1;
			log_buffer[log_buffer_idx] = '\0';
		}
	}

	#endif
}


int log_init(log_user_flusher_fp_t uflusher)
{
	log_user_flusher_fp = uflusher;
	memset(log_buffer, 0, sizeof log_buffer);
	return 0;
}

void log_term(void)
{

}

void log_flush(void)
{
	log_user_flusher_fp(log_buffer);
}


#endif /* LOG_IMPLEMENTATION */





