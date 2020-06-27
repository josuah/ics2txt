#include "ical.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int
ical_read_line(char **line, size_t *sz, FILE *fp)
{
	ssize_t r;
	char *tail = NULL;
	size_t tail_sz = 0;
	int c, ret = -1;

	if ((r = getline(line, sz, fp)) <= 0)
		return r;
	strchomp(*line);

	for (;;) {
		if ((c = fgetc(fp)) == EOF) {
			ret = ferror(fp) ? -1 : 0;
			goto end;
		}
		if (c != ' ')
			break;
		if ((r = getline(&tail, &tail_sz, fp)) <= 0) {
			ret = r;
			goto end;
		}
		strchomp(tail);
		if (strappend(line, tail) < 0)
			goto end;
	}

	ret = 1;
end:
	free(tail);
	ungetc(c, fp);
	return ret;
}

int
ical_parse_contentline(struct ical_contentline *contentline, char *line)
{
	char *column, *equal, *param, *cp;
	size_t sz;

	debug("0");

	if ((column = strchr(line, ':')) == NULL)
		return -1;
	*column = '\0';

	{
		size_t len;

		debug("1.1");
		len = strlen(column + 1);
		debug("1.2");
	}
	

	if ((contentline->value = strdup(column + 1)) == NULL)
		return -1;

	debug("2");

	cp = strchr(line, ';');
	cp = (cp == NULL) ? (NULL) : (cp + 1);

	debug("3");

	while ((param = strsep(&cp, ";")) != NULL) {
		if ((equal = strchr(param, '=')) == NULL)
			return -1;
		*equal = '\0';

		if (map_set(&contentline->param, param, equal + 1) < 0)
			return -1;
	}

	debug("4");

	sz = sizeof(contentline->name);
	if (strlcpy(contentline->name, line, sz) >= sz)
		return errno=EMSGSIZE, -1;

	debug("5");

	return 0;
}

void
ical_init_contentline(struct ical_contentline *contentline)
{
	memset(contentline, 0, sizeof(*contentline));
}


void
ical_free_contentline(struct ical_contentline *contentline)
{
	map_free(&contentline->param);
	free(contentline->value);
}
