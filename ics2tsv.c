#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ical.h"
#include "util.h"

#define FIELDS_MAX 64

typedef struct Field Field;
typedef struct Block Block;

struct Block {
	time_t beg, end;
	char *fields[FIELDS_MAX];
};

Block block;

static int
fn_field_name(IcalParser *p, char *name)
{
	printf("name %s\n", name);
	return 0;
}

static int
fn_block_begin(IcalParser *p, char *name)
{
	debug("begin %s\n", name);
	return 0;
}

static int
fn_block_end(IcalParser *p, char *name)
{
	debug("end %s\n", name);
	return 0;
}

static int
fn_param_value(IcalParser *p, char *name, char *value)
{
	printf("param %s=%s\n", name, value);
	return 0;
}

static int
fn_field_value(IcalParser *p, char *name, char *value)
{
	static char *fieldmap[][2] = {
		[ICAL_BLOCK_VEVENT]	= { "DTSTART",	"DTEND" },
		[ICAL_BLOCK_VTODO]	= { NULL,	"DUE" },
		[ICAL_BLOCK_VJOURNAL]	= { "DTSTAMP",	NULL },
		[ICAL_BLOCK_VFREEBUSY]	= { "DTSTART",	"DTEND" },
		[ICAL_BLOCK_VALARM]	= { "DTSTART",	NULL },
		[ICAL_BLOCK_OTHER]	= { NULL,	NULL },
	};
	char *beg, *end;

	beg = fieldmap[p->blocktype][0];
	if (beg != NULL && strcasecmp(name, beg) == 0)
		if (ical_get_time(p, value, &block.beg) != 0)
			return -1;
	end = fieldmap[p->blocktype][1];
	if (end != NULL && strcasecmp(name, end) == 0)
		if (ical_get_time(p, value, &block.end) != 0)
			return -1;
	return 0;
}

int
main(int argc, char **argv)
{
	IcalParser p = {0};
	arg0 = *argv++;

	p.fn_field_name = fn_field_name;
	p.fn_block_begin = fn_block_begin;
	p.fn_block_end = fn_block_end;
	p.fn_param_value = fn_param_value;
	p.fn_field_value = fn_field_value;

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
