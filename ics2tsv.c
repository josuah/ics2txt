#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ical.h"
#include "util.h"

#define FIELDS_MAX 64

typedef struct Event Event;

struct Event {
	time_t beg, end;
	char *fields[FIELDS_MAX];
};

static char *fields_time[] = {
	"DTSTART", "DTEND", "DTSTAMP", "DUE", "EXDATE", "RDATE"
};

static char *fields_default[] = {
	"ATTENDEE", "CATEGORY", "DESCRIPTION", "LOCATION", "SUMMARY", "URL"
};

static char **fields = fields_default;

static int
fn_entry_name(IcalParser *p, char *name)
{
	printf("name %s\n", name);
	return 0;
}

static int
fn_block_begin(IcalParser *p, char *name)
{
	printf("begin %s\n", name);
	return 0;
}

static int
fn_param_value(IcalParser *p, char *name, char *value)
{
	printf("param %s=%s\n", name, value);
	return 0;
}

static int
fn_entry_value(IcalParser *p, char *name, char *value)
{
	size_t len;
	(void)name;

	if (ical_get_value(p, value, &len) < 0)
		return -1;

	if (strcasecmp(name, "DTSTART") == 0 ||
            strcasecmp(name, "DTSTAMP") == 0 ||
	    strcasecmp(name, "DTEND") == 0) {
		time_t t = 0;
		if (ical_get_time(p, value, &t) != 0)
			warn("%s: %s", p->errmsg, value);
		printf("epoch %lld\n", t);
	} else {	
		printf("value %s\n", value);
	}

	return 0;
}

int
main(int argc, char **argv)
{
	IcalParser p = {0};
	arg0 = *argv++;

	p.fn_entry_name = fn_entry_name;
	p.fn_block_begin = fn_block_begin;
	p.fn_param_value = fn_param_value;
	p.fn_entry_value = fn_entry_value;

	if (*argv == NULL) {
		if (ical_parse(&p, stdin) < 0)
			err("parsing stdin:%d: %s", p.linenum, p.errmsg);
	}

	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;

		debug("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			err("opening %s", *argv);
		if (ical_parse(&p, fp) < 0)
			err("parsing %s:%d: %s", *argv, p.linenum, p.errmsg);
		fclose(fp);
	}
	return 0;
}
