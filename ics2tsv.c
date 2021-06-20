#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include "ical.h"
#include "util.h"

#ifndef __OpenBSD__
#define pledge(...) 0
#endif

#define FIELDS_MAX 128

typedef struct Field Field;
typedef struct Block Block;

struct Field {
	char *key;
	char *value;
};

struct Block {
	time_t beg, end;
	char *fields[FIELDS_MAX];
};

static int flag_header = 1;
static char default_fields[] = "SUMMARY,DESCRIPTION,CATEGORIES,LOCATION";
static char *flag_sep = ",";
static char *flag_timefmt = NULL;
static char *flag_fields = default_fields;
static char *fields[FIELDS_MAX];
static Block block;

static int
fn_field_name(IcalParser *p, char *name)
{
	(void)p;
	(void)name;

	return 0;
}

static int
fn_block_begin(IcalParser *p, char *name)
{
	(void)p;
	(void)name;

	if (p->blocktype == ICAL_BLOCK_OTHER)
		return 0;

	memset(&block, 0, sizeof block);
	return 0;
}

static int
fn_block_end(IcalParser *p, char *name)
{
	(void)name;

	if (p->blocktype == ICAL_BLOCK_OTHER)
		return 0;
	fputs(p->current->name, stdout);

	/* printing dates with %s is much much slower than %lld */
	if (flag_timefmt == NULL) {
		printf("\t%lld\t%lld", block.beg, block.end);
	} else {
		char buf[128];
		struct tm tm = {0};

		localtime_r(&block.beg, &tm);
		strftime(buf, sizeof buf, flag_timefmt, &tm);
		printf("\t%s", buf);

		localtime_r(&block.end, &tm);
		strftime(buf, sizeof buf, flag_timefmt, &tm);
		printf("\t%s", buf);
	}

	/* reserved for recurring events */
	printf("\t%s", "(null)");

	for (int i = 0; fields[i] != NULL; i++) {
		fputc('\t', stdout);
		if (block.fields[i] != NULL)
			fputs(block.fields[i], stdout);
	}
	printf("\n");
	return 0;
}

static int
fn_param_value(IcalParser *p, char *name, char *value)
{
	(void)p;
	(void)name;
	(void)value;

	return 0;
}

static int
fn_field_value(IcalParser *p, char *name, char *value)
{
	static char *map[][2] = {
		[ICAL_BLOCK_VEVENT]	= { "DTSTART",	"DTEND" },
		[ICAL_BLOCK_VTODO]	= { NULL,	"DUE" },
		[ICAL_BLOCK_VJOURNAL]	= { "DTSTAMP",	NULL },
		[ICAL_BLOCK_VFREEBUSY]	= { "DTSTART",	"DTEND" },
		[ICAL_BLOCK_VALARM]	= { "DTSTART",	NULL },
		[ICAL_BLOCK_OTHER]	= { NULL,	NULL },
	};
	char *beg, *end;

	/* fill the date fields */
	beg = map[p->blocktype][0];
	if (beg != NULL && strcasecmp(name, beg) == 0)
		if (ical_get_time(p, value, &block.beg) != 0)
			return -1;
	end = map[p->blocktype][1];
	if (end != NULL && strcasecmp(name, end) == 0)
		if (ical_get_time(p, value, &block.end) != 0)
			return -1;

	/* fill text fields as requested with -o F1,F2... */
	for (int i = 0; fields[i] != NULL; i++) {
		if (strcasecmp(name, fields[i]) == 0) {
			if (block.fields[i] == NULL) {
				if ((block.fields[i] = strdup(value)) == NULL)
					return ical_err(p, strerror(errno));
			} else {
				if (strappend(&block.fields[i], flag_sep) == NULL ||
				    strappend(&block.fields[i], value) == NULL)
					return ical_err(p, strerror(errno));
			}
		}
	}

	return 0;
}

static void
usage(void)
{
	fprintf(stderr,"usage: %s [-1] [-f fields] [-s separator] [-t timefmt]"
	    " [file...]\n", arg0);
	exit(1);
}

int
main(int argc, char **argv)
{
	IcalParser p = {0};
	int c;

	arg0 = *argv;

	if (pledge("stdio rpath", "") < 0)
		err(1, "pledge: %s", strerror(errno));

	p.fn_field_name = fn_field_name;
	p.fn_block_begin = fn_block_begin;
	p.fn_block_end = fn_block_end;
	p.fn_param_value = fn_param_value;
	p.fn_field_value = fn_field_value;

	while ((c = getopt(argc, argv, "01f:s:t:")) != -1) {
		switch (c) {
		case '0':
			flag_header = 0;
			break;
		case '1':
			flag_header = 1;
			break;
		case 'f':
			flag_fields = optarg;
			break;
		case 's':
			flag_sep = optarg;
			break;
		case 't':
			flag_timefmt = optarg;
			break;
		case '?':
			usage();
			break;
		}
	}
	argv += optind;
	argc -= optind;

	if (strsplit(flag_fields, fields, LEN(fields), ",") < 0)
		err(1, "too many fields specified with -f flag");

	if (flag_header) {
		printf("%s\t%s\t%s\t%s", "TYPE", "START", "END", "RECUR");
		for (size_t i = 0; fields[i] != NULL; i++)
			printf("\t%s", fields[i]);
		fputc('\n', stdout);
	}

	if (*argv == NULL || strcmp(*argv, "-") == 0) {
		debug("converting *stdin*");
		if (ical_parse(&p, stdin) < 0)
			err(1, "parsing *stdin*:%d: %s", p.linenum, p.errmsg);
	}
	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;
		debug("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			err(1, "opening %s: %s", *argv, strerror(errno));
		if (ical_parse(&p, fp) < 0)
			err(1, "parsing %s:%d: %s", *argv, p.linenum, p.errmsg);
		fclose(fp);
	}

	return 0;
}
