#include "util.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
strsep(char **str_p, char const *sep)
{
	char *s, *prev;

	if (*str_p == NULL)
		return NULL;

	for (s = prev = *str_p; strchr(sep, *s) == NULL; s++)
		continue;

	if (*s == '\0') {
		*str_p = NULL;
	} else {
		*s = '\0';
		*str_p = s + 1;
	}
	return prev;
}

void
strchomp(char *line)
{
	size_t len;

	len = strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len-- - 1] = '\0';
	if (len > 0 && line[len - 1] == '\r')
		line[len-- - 1] = '\0';
}

int
strappend(char **base_p, char const *s)
{
	size_t base_len, s_len;
	void *v;

	base_len = (*base_p == NULL) ? (0) : (strlen(*base_p));
	s_len = strlen(s);

	if ((v = realloc(*base_p, base_len + s_len + 1)) == NULL)
		return -1;

	*base_p = v;
	memcpy(*base_p + base_len, s, s_len + 1);
	return 0;
}

void *
reallocarray(void *buf, size_t len, size_t sz)
{
	if (SIZE_MAX / len < sz)
		return errno=ERANGE, NULL;
	return realloc(buf, len * sz);
}
