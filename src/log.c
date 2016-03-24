#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdbool.h>

#include "include/log.h"
#include "include/utils.h"

static char * program_name = NULL;

static void to_log(int, const char *, va_list);

void to_log_start(char *progname, bool daemonize){
	program_name = progname;

	openlog(program_name, LOG_PERROR|LOG_PID, daemonize ? LOG_DAEMON : LOG_USER);

	to_log_info("%s - partial file synchronization tool", program_name);
	to_log_info("Starting...");
}

static void to_log(int priority, const char *fmt, va_list args){
	vsyslog(priority, fmt, args);
}

void to_log_err(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	to_log(LOG_ERR, fmt, args);
	va_end(args);
}

void to_log_info(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	to_log(LOG_NOTICE, fmt, args);
	va_end(args);
}

void to_log_warn(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	to_log(LOG_WARNING, fmt, args);
	va_end(args);
}

void to_log_close(){
	to_log_info("Shutting down %s", program_name); 
	closelog();
}

