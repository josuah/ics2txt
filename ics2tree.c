#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ical.h"
#include "util.h"

static void
print_ruler(int level)
{
	while (level-- > 0)
		fprintf(stdout, ": ");
}

static int
fn_entry_name(IcalParser *p, char *name)
{
	print_ruler(p->level);
	printf("name %s\n", name);
	return 0;
}

static int
fn_block_begin(IcalParser *p, char *name)
{
	print_ruler(p->level);
	printf("begin %s\n", name);
	return 0;
}

static int
fn_param_value(IcalParser *p, char *name, char *value)
{
	print_ruler(p->level + 1);
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
	print_ruler(p->level + 1);
	printf("value %s\n", value);
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
			err("parsing stdin:%d: %s", p.line, p.errmsg);
	}

	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;

		debug("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			err("opening %s", *argv);
		if (ical_parse(&p, fp) < 0)
			err("parsing %s:%d: %s", *argv, p.line, p.errmsg);
		fclose(fp);
	}
	return 0;
}
