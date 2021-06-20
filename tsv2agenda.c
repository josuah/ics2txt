#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#ifndef __OpenBSD__
#define pledge(...) 0
#endif

#define FIELDS_MAX 128

enum {
	FIELD_TYPE,
	FIELD_BEG,
	FIELD_END,
	FIELD_RECUR,
	FIELD_OTHER,
};

typedef struct {
	struct tm beg, end;
} AgendaCtx;

static size_t field_categories = 0;
static size_t field_location = 0;
static size_t field_summary = 0;

void
print_date(struct tm *tm)
{
 	if (tm == NULL) {
		fprintf(stdout, "%11s", "");
	} else {
		char buf[128];
		if (strftime(buf, sizeof buf, "%Y-%m-%d", tm) == 0)
			err(1, "strftime: %s", strerror(errno));
		fprintf(stdout, "%s ", buf);
	}
}

void
print_time(struct tm *tm)
{
	if (tm == NULL) {
		fprintf(stdout, "%5s ", "");
	} else {
		char buf[128];
		if (strftime(buf, sizeof buf, "%H:%M", tm) == 0)
			err(1, "strftime: %s", strerror(errno));
		fprintf(stdout, "%5s ", buf);
	}
}

void
print(AgendaCtx *ctx, char **fields, size_t n)
{
	struct tm beg = {0}, end = {0};
	time_t t;
	char const *e;
	int rows, samedate;

	t = strtonum(fields[FIELD_BEG], 0, UINT32_MAX, &e);
	if (e != NULL)
		err(1, "start time %s is %s", fields[FIELD_BEG], e);
	localtime_r(&t, &beg);

	t = strtonum(fields[FIELD_END], 0, UINT32_MAX, &e);
	if (e != NULL)
		err(1, "end time %s is %s", fields[FIELD_END], e);
	localtime_r(&t, &end);

	fputc('\n', stdout);

	samedate = (ctx->beg.tm_year != beg.tm_year || ctx->beg.tm_mon != beg.tm_mon ||
	    ctx->beg.tm_mday != beg.tm_mday);
	print_date(samedate ? &beg : NULL);
	print_time(&beg);

	assert(field_summary < n);
	assert(field_summary > FIELD_OTHER);
	fprintf(stdout, "%s\n", fields[field_summary]);

	samedate = (beg.tm_year != end.tm_year || beg.tm_mon != end.tm_mon ||
	    beg.tm_mday != end.tm_mday);
	print_date(samedate ? &end : NULL);
	print_time(&end);

	rows = 0;

	assert(field_location < n);
	if (field_location > 0 && fields[field_location][0] != '\0') {
		assert(field_summary > FIELD_OTHER);
		fprintf(stdout, "%s\n", fields[field_location]);
		rows++;
	}

	assert(field_categories < n);
	if (field_categories > 0 && fields[field_categories][0] != '\0') {
		assert(field_summary > FIELD_OTHER);
		if (rows > 0) {
			print_date(NULL);
			print_time(NULL);
		}
		fprintf(stdout, "%s\n", fields[field_categories]);
	}

	ctx->beg = beg;
	ctx->end = end;
}

void
set_fields_num(char **fields, size_t n)
{
	struct { char *name; size_t *var; } map[] = {
		{ "CATEGORIES", &field_categories },
		{ "LOCATION", &field_location },
		{ "SUMMARY", &field_summary },
		{ NULL, NULL }
	};

	debug("n=%zd", n);
	for (size_t i1 = FIELD_OTHER; i1 < n; i1++)
		for (size_t i2 = 0; map[i2].name != NULL; i2++)
			if (strcasecmp(fields[i1], map[i2].name) == 0)
				*map[i2].var = i1;
	if (field_summary < FIELD_OTHER)
		err(1, "missing column SUMMARY");
}

ssize_t
tsv_getline(char **fields, size_t max, char **line, size_t *sz, FILE *fp)
{
	char *s;
	size_t n = 0;

	if (getline(line, sz, fp) <= 0)
		return ferror(fp) ? -1 : 0;
	s = *line;
	strchomp(s);

	do {
		if (n >= max)
			return errno=E2BIG, -1;
	} while ((fields[n++] = strsep(&s, "\t")) != NULL);

	return n - 1;
}

int
main(int argc, char **argv)
{
	AgendaCtx ctx = {0};
	ssize_t nfield, n;
	size_t sz = 0;
	char *line = NULL, *fields[FIELDS_MAX];

	arg0 = *argv;

	if (pledge("stdio", "") < 0)
		err(1, "pledge: %s", strerror(errno));

	nfield = tsv_getline(fields, FIELDS_MAX, &line, &sz, stdin);
	if (nfield == -1)
		err(1, "reading stdin: %s", strerror(errno));
	if (nfield == 0)
		err(1, "empty input");
	if (nfield < FIELD_OTHER)
		err(1, "not enough input columns");

	set_fields_num(fields, nfield);

	for (size_t num = 1;; num++) {
		n = tsv_getline(fields, FIELDS_MAX, &line, &sz, stdin);
		if (n < 0)
			err(1, "line %zd: reading stdin: %s", num, strerror(errno));
		if (n == 0)
			break;
		if (n != nfield)
			err(1, "line %zd: had %lld columns, wanted %lld",
			    num, n, nfield);

		print(&ctx, fields, n);
	}
	fputc('\n', stdout);

	free(line);

	return 0;
}
