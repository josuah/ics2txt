#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "ical.h"
#include "util.h"

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

static int flag_1 = 0;
static char default_fields[] = "CATEGORIES,LOCATION,SUMMARY,DESCRIPTION";
static char *flag_s = ",";
static char *flag_t = NULL;
static char *flag_f = default_fields;
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
	char buf[128];
	struct tm tm = {0};

	(void)name;

	if (p->blocktype == ICAL_BLOCK_OTHER)
		return 0;
	fputs(p->current->name, stdout);

	/* printing dates with %s is much much slower than %lld */
	if (flag_t == NULL) {
		printf("\t%lld\t%lld", block.beg, block.end);
	} else {
		strftime(buf, sizeof buf, flag_t, gmtime_r(&block.beg, &tm));
		printf("\t%s", buf);
		strftime(buf, sizeof buf, flag_t, gmtime_r(&block.end, &tm));
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
				if (strappend(&block.fields[i], flag_s) == NULL ||
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
	fprintf(stderr,"usage: %s [-1] [-f fields] [-s subsep] [-t timefmt]"
	    " [file...]\n", arg0);
	exit(1);
}

int
main(int argc, char **argv)
{
	IcalParser p = {0};
	size_t i;
	int c;

	arg0 = *argv;

	p.fn_field_name = fn_field_name;
	p.fn_block_begin = fn_block_begin;
	p.fn_block_end = fn_block_end;
	p.fn_param_value = fn_param_value;
	p.fn_field_value = fn_field_value;

	while ((c = getopt(argc, argv, "1f:s:t:")) != -1) {
		switch (c) {
		case '1':
			flag_1 = 1;
			break;
		case 'f':
			flag_f = optarg;
			break;
		case 's':
			flag_s = optarg;
			break;
		case 't':
			flag_t = optarg;
			break;
		case '?':
			usage();
			break;
		}
	}
	argv += optind;
	argc -= optind;

	i = 0;
	do {
		if (i >= sizeof fields / sizeof *fields - 1)
			err("too many fields specified with -o flag");
	} while ((fields[i++] = strsep(&flag_f, ",")) != NULL);
	fields[i] = NULL;

	if (flag_1) {
		printf("%s\t%s\t%s", "TYPE", "BEG", "END");
		for (i = 0; fields[i] != NULL; i++)
			printf("\t%s", fields[i]);
		fputc('\n', stdout);
	}

	if (*argv == NULL || strcmp(*argv, "-") == 0) {
		debug("converting *stdin*");
		if (ical_parse(&p, stdin) < 0)
			err("parsing *stdin*:%d: %s", p.linenum, p.errmsg);
	}
	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;
		debug("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			err("opening %s: %s", *argv, strerror(errno));
		if (ical_parse(&p, fp) < 0)
			err("parsing %s:%d: %s", *argv, p.linenum, p.errmsg);
		fclose(fp);
	}
	return 0;
}
