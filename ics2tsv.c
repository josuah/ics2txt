#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ical.h"
#include "log.h"
#include "util.h"

int
print_ical_tsv(FILE *fp)
{
	struct ical_vcalendar vcal;
	int e;

	if ((e = ical_read_vcalendar(&vcal, fp)) < 0)
		die("reading ical file: %s", ical_strerror(e));

	ical_free_vcalendar(&vcal);
	return 0;
}

void
print_header(void)
{
	char *fields[] = { "", NULL };

	printf("%s\t%s", "beg", "end");

	for (char **f = fields; *f != NULL; f++) {
		fprintf(stdout, "\t%s", *f);
	}
	fprintf(stdout, "\n");
}

int
main(int argc, char **argv)
{
	print_header();

	log_arg0 = *argv++;

	if (*argv == NULL) {
		if (print_ical_tsv(stdin) < 0)
			die("converting stdin");
	}

	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;

		info("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			die("opening %s", *argv);
		if (print_ical_tsv(fp) < 0)
			die("converting %s", *argv);
		fclose(fp);
	}

	return 0;
}
