#include <windows.h>
#include <string.h>
#include "utils.h"
#include "error.h"


#define LAST_ERROR_BUFSZ (196)

const char issue_reporting_str[] = "\nIssue report: github.com/dhustkoder/wiilinku\0";
char last_error_str[LAST_ERROR_BUFSZ + sizeof(issue_reporting_str)];


void set_last_error(const char* message)
{
	size_t len = strlen(message);
	
	if (len > LAST_ERROR_BUFSZ)
		len = LAST_ERROR_BUFSZ;

	memcpy(last_error_str, message, len);
	memcpy(last_error_str + len, issue_reporting_str, sizeof(issue_reporting_str));
}

const char* get_last_error(void)
{
	return last_error_str;
}




