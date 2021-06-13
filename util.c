#include "util.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *arg0;

/* logging */

static void
_log(char const *tag, char const *fmt, va_list va)
{
	if (arg0 != NULL)
		fprintf(stderr, "%s: ", arg0);
	fprintf(stderr, "%s: ", tag);
	vfprintf(stderr, fmt, va);
	if (errno != 0)
		fprintf(stderr, ": %s", strerror(errno));
	fprintf(stderr, "\n");
	fflush(stderr);
}

void
err(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	_log("error", fmt, va);
	exit(1);
}

void
warn(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	_log("warning", fmt, va);
}

void
debug(char const *fmt, ...)
{
	static int verbose = -1;
	va_list va;

	if (verbose < 0)
		verbose = (getenv("DEBUG") != NULL);
	if (!verbose)
		return;
	va_start(va, fmt);
	_log("debug", fmt, va);
}

/* strings */

size_t
strlcpy(char *buf, char const *str, size_t sz)
{
	size_t len, cpy;

	len = strlen(str);
	cpy = (len > sz) ? (sz) : (len);
	memcpy(buf, str, cpy + 1);
	buf[sz - 1] = '\0';
	return len;
}

char *
strsep(char **sp, char const *sep)
{
	char *s, *prev;

	if (*sp == NULL)
		return NULL;
	prev = *sp;
	for (s = *sp; strchr(sep, *s) == NULL; s++)
		continue;
	if (*s == '\0') {
		*sp = NULL;
	} else {
		*sp = s + 1;
		*s = '\0';
	}
	return prev;
}

void
strchomp(char *line)
{
	size_t len;

	len = strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[--len] = '\0';
	if (len > 0 && line[len - 1] == '\r')
		line[--len] = '\0';
}

int
strappend(char **dstp, char const *src)
{
	size_t dstlen, srclen;
	void *mem;

	dstlen = (*dstp == NULL) ? 0 : strlen(*dstp);
	srclen = strlen(src);

	if ((mem = realloc(*dstp, dstlen + srclen + 1)) == NULL)
		return -1;
	*dstp = mem;

	memcpy(*dstp + dstlen, src, srclen + 1);
	return 0;
}

/* memory */

void *
reallocarray(void *buf, size_t len, size_t sz)
{
	if (SIZE_MAX / len < sz)
		return errno=ERANGE, NULL;
	return realloc(buf, len * sz);
}
