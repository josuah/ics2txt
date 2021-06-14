#include "ical.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "util.h"
#include "base64.h"

#define Xstrlcpy(d, s) (strlcpy((d), (s), sizeof(d)) < sizeof(d))
#define Xstrlcat(d, s) (strlcat((d), (s), sizeof(d)) < sizeof(d))

/* helpers: common utilities to call within the p->fn() callbacks as
 * well as in the code below */

int
ical_error(IcalParser *p, char const *msg)
{
	p->errmsg = msg;
	return -1;
}

int
ical_get_level(IcalParser *p)
{
	return p->current - p->stack;
}

int
ical_get_value(IcalParser *p, char *s, size_t *len)
{
	*len = strlen(s);
	if (p->base64)
		if (base64_decode(s, len, s, len) < 0)
			return ical_error(p, "invalid base64 data");
	return 0;
}

int
ical_get_time(IcalParser *p, char *s, time_t *t)
{
	struct tm tm = {0};
	char const *tzid;

	tzid = (p->tzid) ? p->tzid :
	    (p->current && p->current->tzid[0] != '\0') ? p->current->tzid :
	    "";

#define N(i, x) ((s[i] - '0') * x)

	/* date */
	for (int i = 0; i < 8; i++)
		if (!isdigit(s[i]))
			return ical_error(p, "invalid date format");
	tm.tm_year = N(0,1000) + N(1,100) + N(2,10) + N(3,1) - 1900;
	tm.tm_mon = N(4,10) + N(5,1) - 1;
	tm.tm_mday = N(6,10) + N(7,1);
	s += 8;

	if (*s == 'T') {
		/* time */
		s++;
		for (int i = 0; i < 6; i++)
			if (!isdigit(s[i]))
				return ical_error(p, "invalid time format");
		tm.tm_hour = N(0,10) + N(1,1);
		tm.tm_min = N(2,10) + N(3,1);
		tm.tm_sec = N(4,10) + N(5,1);
		if (s[6] == 'Z')
			tzid = "UTC";
	}

	if ((*t = tztime(&tm, tzid)) == (time_t)-1)
		return ical_error(p, "could not convert time");

	return 0;
}

/* hooks: called just before user functions to do extra work such as
 * processing time zones definition or prepare base64 decoding, and
 * permit to only have parsing code left to parsing functions */

int
hook_entry_name(IcalParser *p, char *name)
{
	(void)p; (void)name;
	return 0;
}

int
hook_param_name(IcalParser *p, char *name)
{
	(void)p; (void)name;
	return 0;
}

int
hook_param_value(IcalParser *p, char *name, char *value)
{
	if (strcasecmp(name, "ENCODING") == 0)
		p->base64 = (strcasecmp(value, "BASE64") == 0);

	if (strcasecmp(name, "TZID") == 0)
		p->tzid = value;

	return 0;
}

int
hook_entry_value(IcalParser *p, char *name, char *value)
{
	if (strcasecmp(name, "TZID") == 0)
		if (!Xstrlcpy(p->current->tzid, value))
			return ical_error(p, "TZID: name too large");

	p->tzid = NULL;

	return 0;
}

int
hook_block_begin(IcalParser *p, char *name)
{
	p->current++;
	memset(p->current, 0, sizeof(*p->current));
	if (ical_get_level(p) >= ICAL_STACK_SIZE)
		return ical_error(p, "max recurion reached");
	if (!Xstrlcpy(p->current->name, name))
		return ical_error(p, "value too large");
	return 0;
}

int
hook_block_end(IcalParser *p, char *name)
{
	if (strcasecmp(p->current->name, name) != 0)
		return ical_error(p, "mismatching BEGIN: and END:");
	p->current--;
	if (p->current < p->stack)
		return ical_error(p, "more END: than BEGIN:");
	return 0;
}

/* parsers: in charge of reading from `fp`, splitting text into
 * fields, and call hooks and user functions. */

#define CALL(p, fn, ...) ((p)->fn ? (p)->fn((p), __VA_ARGS__) : 0)

static int
ical_parse_value(IcalParser *p, char **sp, char *name)
{
	int err;
	char *s, c, *val;

	s = *sp;
	if (*s == '"') {
		val = ++s;
		while (!iscntrl(*s) && *s != '"')
			s++;
		if (*s != '"')
			return ical_error(p, "missing '\"'");
		*s++ = '\0';
	} else {
		val = s;
		while (!iscntrl(*s) && !strchr(",;:'\"", *s))
			s++;
	}
	c = *s, *s = '\0';
	if ((err = hook_param_value(p, name, val)) != 0 ||
	    (err = CALL(p, fn_param_value, name, val)) != 0)
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
		if ((err = hook_param_name(p, name)) != 0 ||
		    (err = CALL(p, fn_param_name, name)) != 0)
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
ical_parse_contentline(IcalParser *p, char *s)
{
	int err;
	char c, *name, *sep;

	for (name = s; isalnum(*s) || *s == '-'; s++);
	if (s == name || (*s != ';' && *s != ':'))
		return ical_error(p, "invalid entry name");
	c = *s, *s = '\0';
	if (strcasecmp(name, "BEGIN") != 0 && strcasecmp(name, "END") != 0)
		if ((err = hook_entry_name(p, name)) != 0 ||
		    (err = CALL(p, fn_entry_name, name)) != 0)
			return err;
	*s = c;
	sep = s;

	p->base64 = 0;
	while (*s == ';') {
		s++;
		if ((err = ical_parse_param(p, &s)) != 0)
			return err;
	}

	if (*s != ':')
		return ical_error(p, "expected ':' delimiter");
	s++;

	*sep = '\0';
	if (strcasecmp(name, "BEGIN") == 0) {
		if ((err = hook_block_begin(p, s)) != 0 ||
		    (err = CALL(p, fn_block_begin, s)) != 0)
			return err;
	} else if (strcasecmp(name, "END") == 0) {
		if ((err = hook_block_end(p, s)) != 0 ||
		    (err = CALL(p, fn_block_end, s)) != 0)
			return err;
	} else {
		if ((err = hook_entry_value(p, name, s)) != 0 ||
		    (err = CALL(p, fn_entry_value, name, s)) != 0)
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

	p->current = p->stack;

	while (!feof(fp)) {
		if ((contentline = realloc(contentline, 1)) == NULL)
			return ical_error(p, strerror(errno));
		*contentline = '\0';

		do {
			do {
				p->linenum++;
				if (getline(&ln, &sz, fp) <= 0) {
					if (ferror(fp))
						return ical_error(p, strerror(errno));
					goto end;
				}
				strchomp(ln);
			} while (*ln == '\0');

			if (strappend(&contentline, ln) < 0)
				return ical_error(p, strerror(errno));
			if ((c = fgetc(fp)) == EOF) {
				if (ferror(fp))
					return ical_error(p, strerror(errno));
				goto done;
			}
		} while (c == ' ');
		ungetc(c, fp);
done:
		assert(!ferror(fp));
		if ((err = ical_parse_contentline(p, contentline)) != 0)
			break;
	}
end:
	free(contentline);
	free(ln);
	return err;
}
