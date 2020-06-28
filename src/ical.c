#include "ical.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* strcase* */

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
	assert(!ferror(fp));
	return 1;
}

int
ical_parse_contentline(struct ical_contentline *cl, char *line)
{
	char *column, *equal, *param, *cp;
	size_t sz;
	int e = errno;

	if ((column = strchr(line, ':')) == NULL)
		return -1;
	*column = '\0';
	if ((cl->value = strdup(column + 1)) == NULL)
		return -1;

	if ((cp = strchr(line, ';')) != NULL)
		cp++;
	while ((param = strsep(&cp, ";")) != NULL) {
		if ((equal = strchr(param, '=')) == NULL)
			return -1;
		*equal = '\0';
		if (map_set(&cl->param, param, equal + 1) < 0)
			return -1;
	}

	sz = sizeof cl->name;
	if (strlcpy(cl->name, line, sz) >= sz)
		return errno=EMSGSIZE, -1;

	assert(errno == e);
	return 0;
}

int
ical_parse_tzid(struct ical_value *value, struct ical_contentline *cl)
{
	return 0;
}

int
ical_parse_date(struct ical_value *value, struct ical_contentline *cl)
{
	return 0;
}

int
ical_parse_attribute(struct ical_value *value, struct ical_contentline *cl)
{
	return 0;
}

int
ical_begin_vnode(struct ical_vcalendar *vcal, char const *name)
{
	if (strcasecmp(name, "VCALENDAR"))
		return 0;
	return -1;
}

int
ical_end_vnode(struct ical_vcalendar *vcal, char const *name)
{
	if (strcasecmp(name, "VCALENDAR"))
		return 0;
	return -1;
}

int
ical_add_contentline(struct ical_vcalendar *vcal, struct ical_contentline *cl)
{
	struct ical_value value_buf, *value = &value_buf;
	int i;
	struct {
		char *name;
		enum ical_value_type type;
		int (*fn)(struct ical_value *, struct ical_contentline *);
	} map[] = {
		{ "DTSTART", ICAL_VALUE_TIME,      ical_parse_date },
		{ "DTEND",   ICAL_VALUE_TIME,      ical_parse_date },
		{ "TZID",    ICAL_VALUE_TIME,      ical_parse_tzid },
		{ NULL,      ICAL_VALUE_ATTRIBUTE, ical_parse_attribute },
	};

	if (strcasecmp(cl->name, "BEGIN") == 0)
		return ical_begin_vnode(vcal, cl->value);

	if (strcasecmp(cl->name, "END") == 0)
		return ical_end_vnode(vcal, cl->value);

	memset(value, 0, sizeof *value);

	for (i = 0; map[i].name == NULL; i++)
		if (strcasecmp(cl->name, map[i].name) == 0)
			break;
	value->type = map[i].type;
	if (map[i].fn(value, cl) < 0)
		return -1;
	return 0;
}

void
ical_free_value(struct ical_value *value)
{
	;
}

void
ical_free_contentline(struct ical_contentline *cl)
{
	map_free(&cl->param);
	free(cl->value);
}
