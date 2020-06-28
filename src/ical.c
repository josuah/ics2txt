#include "ical.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int
ical_read_line(char **line, char **ln, size_t *sz, FILE *fp)
{
	int c;
	void *v;

	if ((v = realloc(*line, 1)) == NULL)
		return -1;
	*line = v;
	(*line)[0] = '\0';

	do {
		if (getline(ln, sz, fp) <= 0)
			return ferror(fp) ? -1 : 0;
		strchomp(*ln);
		if (strappend(line, *ln) < 0)
			return -1;
		if ((c = fgetc(fp)) == EOF)
			return ferror(fp) ? -1 : 1;
	} while (c == ' ');

	ungetc(c, fp);
	return 1;
}

int
ical_parse_contentline(struct ical_contentline *contentline, char *line)
{
	char *column, *equal, *param, *cp;
	size_t sz;

	if ((column = strchr(line, ':')) == NULL)
		return -1;
	*column = '\0';
	if ((contentline->value = strdup(column + 1)) == NULL)
		return -1;

	if ((cp = strchr(line, ';')) != NULL)
		cp++;
	while ((param = strsep(&cp, ";")) != NULL) {
		if ((equal = strchr(param, '=')) == NULL)
			return -1;
		*equal = '\0';
		if (map_set(&contentline->param, param, equal + 1) < 0)
			return -1;
	}

	sz = sizeof(contentline->name);
	if (strlcpy(contentline->name, line, sz) >= sz)
		return errno=EMSGSIZE, -1;

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
