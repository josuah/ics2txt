#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ical.h"
#include "util.h"

static void
print_ruler(int level)
{
	while (level-- > 0)
		fprintf(stdout, ": ");
}

static int
fn_field_name(IcalParser *p, char *name)
{
	print_ruler(ical_get_level(p));
	printf("name %s\n", name);
	fflush(stdout);
	return 0;
}

static int
fn_block_begin(IcalParser *p, char *name)
{
	print_ruler(ical_get_level(p) - 1);
	printf("begin %s\n", name);
	fflush(stdout);
	return 0;
}

static int
fn_param_value(IcalParser *p, char *name, char *value)
{
	print_ruler(ical_get_level(p) + 1);
	printf("param %s=%s\n", name, value);
	fflush(stdout);
	return 0;
}

static int
fn_field_value(IcalParser *p, char *name, char *value)
{
	size_t len;
	(void)name;

	if (ical_get_value(p, value, &len) < 0)
		return -1;
	print_ruler(ical_get_level(p) + 1);
	if (strcasecmp(name, "DTSTART") == 0 ||
            strcasecmp(name, "DTSTAMP") == 0 ||
	    strcasecmp(name, "DTEND") == 0) {
		time_t t;
		if (ical_get_time(p, value, &t) != 0)
			warn("%s: %s", p->errmsg, value);
		printf("epoch %lld\n", t);
	} else {	
		printf("value %s\n", value);
	}
	fflush(stdout);
	return 0;
}

int
main(int argc, char **argv)
{
	IcalParser p = {0};
	arg0 = *argv++;

	p.fn_field_name = fn_field_name;
	p.fn_block_begin = fn_block_begin;
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
