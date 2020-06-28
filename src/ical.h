#ifndef ICAL_H
#define ICAL_H

#include <stdio.h>
#include <time.h>

#include "map.h"

struct ical_vevent {
	time_t beg, end;
	struct map map;
};

struct ical_contentline {
	char name[32], *value;
	struct map param;
};

/** src/ical.c **/
int ical_read_line(char **line, char **ln, size_t *sz, FILE *fp);
int ical_parse_contentline(struct ical_contentline *contentline, char *line);
void ical_init_contentline(struct ical_contentline *contentline);
void ical_free_contentline(struct ical_contentline *contentline);

#endif
