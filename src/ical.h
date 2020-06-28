#ifndef ICAL_H
#define ICAL_H

#include <stdio.h>
#include <time.h>

#include "map.h"

#define ICAL_NEST_MAX 4

/*  */

struct ical_contentline {
	char name[32], *value;
	struct map param;
};

/* single value for an iCalendar element attribute */

enum ical_value_type {
	ICAL_VALUE_TIME, ICAL_VALUE_ATTRIBUTE,
} type;

union ical_value_union {
	time_t *time;
	char *str;
};

struct ical_value {
	enum ical_value_type type;
	union ical_value_union value;
};

/* global propoerties for an iCalendar document as well as parsing state */

struct ical_vcalendar {
	time_t tzid;
	char *stack[ICAL_NEST_MAX + 1];
	struct ical_vnode *current;
};

/* element part of an iCalendar document with eventual nested childs */

struct ical_vnode {
	char name[32];
	time_t beg, end;
	struct map properties; /* struct ical_value */
	struct ical_vnode *child, *next;
};

/** src/ical.c **/
int ical_read_line(char **line, char **ln, size_t *sz, FILE *fp);
int ical_parse_contentline(struct ical_contentline *contentline, char *line);
void ical_init_contentline(struct ical_contentline *contentline);
void ical_free_contentline(struct ical_contentline *contentline);

#endif
