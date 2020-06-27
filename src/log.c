#include "log.h"

#include <assert.h>
#include <string.h>

/*
 * log.c - log to standard error according to the log level
 *
 * Instead of logging to syslog, delegate logging to a separate
 * tool, such as FreeBSD's daemon(8), POSIX's logger(1).
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_DEFAULT 3 /* info */

int log_level = -1;
char *log_arg0 = NULL;

void
vlogf(int level, char const *flag, char const *fmt, va_list va)
{
	char *env;
	int e = errno;

	if (log_level < 0) {
		env = getenv("LOG");
		log_level = (env == NULL ? 0 : atoi(env));
		log_level = (log_level > 0 ? log_level : LOG_DEFAULT);
	}

	if (log_level < level)
		return;

	if (log_arg0 != NULL)
		fprintf(stderr, "%s: ", log_arg0);

	fprintf(stderr, "%s: ", flag);
	vfprintf(stderr, fmt, va);

	if (e != 0)
		fprintf(stderr, ": %s", strerror(e));

	fprintf(stderr, "\n");
	fflush(stderr);
}

void
die(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vlogf(1, "error", fmt, va);
	va_end(va);
	exit(1);
}

void
warn(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vlogf(2, "warn", fmt, va);
	va_end(va);
}

void
info(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vlogf(3, "info", fmt, va);
	va_end(va);
}

void
debug(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vlogf(4, "debug", fmt, va);
	va_end(va);
}
