#include "util.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

char *arg0;

static void
_log(char const *fmt, va_list va)
{
	if (arg0 != NULL)
		fprintf(stderr, "%s: ", arg0);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(stderr);
}

void
err(int e, char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	_log( fmt, va);
	exit(e);
}

void
warn(char const *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	_log(fmt, va);
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
	_log(fmt, va);
}

size_t
strlcpy(char *d, char const *s, size_t sz)
{
	size_t len, cpy;

	len = strlen(s);
	cpy = (len > sz) ? (sz) : (len);
	memcpy(d, s, cpy + 1);
	d[sz - 1] = '\0';
	return len;
}

size_t
strlcat(char *d, char const *s, size_t dsz)
{
	size_t dlen;

	dlen = strlen(d);
	if (dlen >= dsz)
		return dlen + strlen(s);
	return dlen + strlcpy(d + dlen, s, dsz - dlen);
}

char *
strsep(char **sp, char const *sep)
{
	char *s, *prev;

	if (*sp == NULL)
		return NULL;
	prev = *sp;
	for (s = *sp; strchr(sep, *s) == NULL; s++);
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

char *
strappend(char **dp, char const *s)
{
	size_t dlen, slen;
	void *mem;

	dlen = (*dp == NULL) ? 0 : strlen(*dp);
	slen = strlen(s);
	if ((mem = realloc(*dp, dlen + slen + 1)) == NULL)
		return NULL;
	*dp = mem;
	memcpy(*dp + dlen, s, slen + 1);
	return *dp;
}

size_t
strsplit(char *s, char **array, size_t len, char const *sep)
{
	size_t i;

	assert(len > 0);
	for (i = 0; i < len; i++)
		if ((array[i] = strsep(&s, sep)) == NULL)
			break;
	array[len - 1] = NULL;
	return i;
}

long long
strtonum(char const *s, long long min, long long max, char const **errstr)
{
	long long ll = 0;
	char *end;

	assert(min < max);
	errno = 0;
	ll = strtoll(s, &end, 10);
	if ((errno == ERANGE && ll == LLONG_MIN) || ll < min) {
		if (errstr != NULL)
			*errstr = "too small";
		return 0;
	}
	if ((errno == ERANGE && ll == LLONG_MAX) || ll > max) {
		if (errstr != NULL)
			*errstr = "too large";
		return 0;
	}
	if (errno == EINVAL || *end != '\0') {
		if (errstr != NULL)
			*errstr = "invalid";
		return 0;
	}
	assert(errno == 0);
	if (errstr != NULL)
		*errstr = NULL;
	return ll;
}

void *
reallocarray(void *mem, size_t n, size_t sz)
{
	if (SIZE_MAX / n < sz)
		return errno=ERANGE, NULL;
	return realloc(mem, n * sz);
}

time_t
tztime(struct tm *tm, char const *tz)
{
	char *env, old[32];
	time_t t;

	env = getenv("TZ");
	if (strlcpy(old, env ? env : "", sizeof old) >= sizeof old)
		return -1;
	if (setenv("TZ", tz, 1) < 0)
		return -1;

	tzset();
	t = mktime(tm);

	if (env == NULL)
		unsetenv("TZ");
	else if (setenv("TZ", old, 1) < 0)
		return -1;
	return t;
}
