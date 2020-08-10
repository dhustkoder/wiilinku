#include <windows.h>
#include <string.h>
#include "utils.h"
#include "error.h"

#define ISSUE_REPORTING_STR_LEN (45)
#define LAST_ERROR_BUFSZ (196)

const char issue_reporting_str[ISSUE_REPORTING_STR_LEN + 1] = ("\nIssue report: github.com/dhustkoder/wiilinku");
char last_error_str[LAST_ERROR_BUFSZ +  ISSUE_REPORTING_STR_LEN + 1];


void set_last_error(const char* message)
{
	WLU_ASSERT(strlen(issue_reporting_str) == ISSUE_REPORTING_STR_LEN);

	size_t len = strlen(message);
	
	if (len > LAST_ERROR_BUFSZ) {
		len = LAST_ERROR_BUFSZ;
	}

	memcpy(last_error_str, message, len);
	memcpy(last_error_str + len, issue_reporting_str, ISSUE_REPORTING_STR_LEN);

	last_error_str[len + ISSUE_REPORTING_STR_LEN] = '\0';
}

const char* get_last_error(void)
{
	return last_error_str;
}




