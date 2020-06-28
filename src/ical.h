#ifndef ICAL_H
#define ICAL_H

#include <stdio.h>
#include <time.h>

#include "map.h"

#define ICAL_NESTED_MAX 4

enum ical_err {
	ICAL_ERR_OK,
	ICAL_ERR_SYSTEM,
	ICAL_ERR_END_MISMATCH,
	ICAL_ERR_MISSING_BEGIN,
	ICAL_ERR_MISSING_COLUMN,
	ICAL_ERR_MISSING_SEMICOLUMN,
	ICAL_ERR_MISSING_EQUAL,
	ICAL_ERR_MIN_NESTED,
	ICAL_ERR_MAX_NESTED,

	ICAL_ERR_LENGTH,
};

/* global propoerties for an iCalendar document as well as parsing state */

struct ical_vcalendar {
	time_t tzid;
	struct ical_vnode *root;
	struct ical_vnode *nested[ICAL_NESTED_MAX + 1];
	struct ical_vnode *current;
};

/* element part of an iCalendar document with eventual nested childs */

struct ical_vnode {
	char name[32];
	time_t beg, end;
	struct map values; /*(struct ical_value *)*/
	struct map child; /*(struct ical_vnode *)*/
	struct ical_vnode *next;
};

/* one line whith the whole content unfolded */

struct ical_value {
	char *name, *value;
	struct map param;
	struct ical_value *next;
	char buf[];
};

/** src/ical.c **/
int ical_getline(char **line, char **ln, size_t *sz, FILE *fp);
char * ical_strerror(int i);
struct ical_value * ical_new_value(char const *line);
void ical_free_value(struct ical_value *value);
int ical_parse_value(struct ical_value *value);
struct ical_vnode * ical_new_vnode(char const *name);
void ical_free_vnode(struct ical_vnode *node);
int ical_push_nested(struct ical_vcalendar *vcal, struct ical_vnode *new);
struct ical_vnode * ical_pop_nested(struct ical_vcalendar *vcal);
int ical_begin_vnode(struct ical_vcalendar *vcal, char const *name);
int ical_end_vnode(struct ical_vcalendar *vcal, char const *name);
int ical_push_value(struct ical_vcalendar *vcal, struct ical_value *new);
void ical_free_vcalendar(struct ical_vcalendar *vcal);
int ical_read_vcalendar(struct ical_vcalendar *vcal, FILE *fp);

#endif
