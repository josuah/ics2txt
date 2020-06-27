#include <stdio.h>

#include "ical.h"
#include "log.h"
#include "util.h"

int
print_ical_to_tsv(FILE *fp)
{
	struct ical_contentline contentline;
	char *line = NULL;
	size_t sz = 0;
	ssize_t r;

	ical_init_contentline(&contentline);

	while ((r = ical_read_line(&line, &sz, fp)) > 0) {
		debug("readling line \"%s\"", line);
		if (ical_parse_contentline(&contentline, line) < 0)
			die("parsing line \"%s\"", line);
	}
	return r;
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
		if (print_ical_to_tsv(stdin) < 0)
			die("converting stdin");
	}

	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;

		info("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			die("opening %s", *argv);
		if (print_ical_to_tsv(fp) < 0)
			die("converting %s", *argv);
		fclose(fp);
	}

	return 0;
}
