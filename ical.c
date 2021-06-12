#include "ical.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "util.h"

static int
ical_error(IcalParser *p, char const *msg)
{
	p->errmsg = msg;
	return -1;
}

#define CALL(p, fn, ...) ((p)->fn ? (p)->fn((p), __VA_ARGS__) : 0)

static int
ical_parse_value(IcalParser *p, char **sp, char *name)
{
	int err;
	char *s, c, *val;

	s = *sp;

	if (*s == '"') {
		++s;
		for (val = s; !iscntrl(*s) && !strchr(",;:\"", *s); s++);
		if (*s != '"')
			return ical_error(p, "missing '\"'");
		*s++ = '\0';
	} else {
		for (val = s; !iscntrl(*s) && !strchr(",;:'\"", *s); s++);
	}

	c = *s, *s = '\0';
	if ((err = CALL(p, fn_param_value, name, val)) != 0)
		return err;
	*s = c;

	*sp = s;
	return 0;
}

static int
ical_parse_param(IcalParser *p, char **sp)
{
	int err;
	char *s, *name;

	s = *sp;

	do {
		for (name = s; isalnum(*s) || *s == '-'; s++);
		if (s == name || (*s != '='))
			return ical_error(p, "invalid parameter name");
		*s++ = '\0';
		if ((err = CALL(p, fn_param_name, name)) != 0)
			return err;

		do {
			if ((err = ical_parse_value(p, &s, name)) != 0)
				return err;
		} while (*s == ',' && s++);
	} while (*s == ';' && s++);

	*sp = s;
	return 0;
}

static int
ical_parse_contentline(IcalParser *p, char *line)
{
	int err;
	char *s, c, *name, *end;

	s = line;

	for (name = s; isalnum(*s) || *s == '-'; s++);
	if (s == name || (*s != ';' && *s != ':'))
		return ical_error(p, "invalid entry name");
	c = *s, *s = '\0';
	if (strcasecmp(name, "BEGIN") != 0 && strcasecmp(name, "END") != 0)
		if ((err = CALL(p, fn_entry_name, name)) != 0)
			return err;
	*s = c;
	end = s;

	while (*s == ';') {
		s++;
		if ((err = ical_parse_param(p, &s)) != 0)
			return err;
	}

	if (*s != ':')
		return ical_error(p, "expected ':' delimiter");
	s++;

	*end = '\0';
	if (strcasecmp(name, "BEGIN") == 0) {
		if ((err = CALL(p, fn_block_begin, s)) != 0)
			return err;
		p->level++;
	} else if (strcasecmp(name, "END") == 0) {
		if ((err = CALL(p, fn_block_end, s)) != 0)
			return err;
		p->level--;
	} else {
		if ((err = CALL(p, fn_entry_value, name, s)) != 0)
			return err;
	}

	return 0;
}

int
ical_parse(IcalParser *p, FILE *fp)
{
	char *ln = NULL, *contentline = NULL;
	size_t sz = 0;
	int err, c;

	while (!feof(fp)) {
		if ((contentline = realloc(contentline, 1)) == NULL)
			return -1;
		*contentline = '\0';

		do {
			do {
				p->line++;
				if (getline(&ln, &sz, fp) <= 0)
					return -1;
				strchomp(ln);
			} while (*ln == '\0');

			if (strappend(&contentline, ln) < 0)
				return -1;
			if ((c = fgetc(fp)) == EOF) {
				if (ferror(fp))
					return -1;
				goto done;
			}
		} while (c == ' ');
		ungetc(c, fp);
done:
		assert(!ferror(fp));
		if ((err = ical_parse_contentline(p, contentline)) != 0)
			break;
	}
	free(contentline);
	free(ln);
	return err;
}
