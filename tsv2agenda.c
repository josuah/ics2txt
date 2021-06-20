#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "util.h"

#ifndef __OpenBSD__
#define pledge(...) 0
#endif

enum {
	FIELD_TYPE,
	FIELD_BEG,
	FIELD_END,
	FIELD_RECUR,
	FIELD_OTHER,
	FIELD_MAX = 128,
};

typedef struct {
	struct tm beg, end;
	char *fieldnames[FIELD_MAX];
	size_t fieldnum;
	size_t linenum;
} AgendaCtx;

static time_t flag_from = INT64_MIN;
static time_t flag_to = INT64_MAX;

static void
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

static void
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

static void
print_header0(struct tm *old, struct tm *new)
{
	int same;

	same = (old->tm_year == new->tm_year && old->tm_mon == new->tm_mon &&
	    old->tm_mday == new->tm_mday);
	print_date(same ? NULL : new);
	print_time(new);
}

static void
print_header1(struct tm *beg, struct tm *end)
{
	int same;

	same = (beg->tm_year == end->tm_year && beg->tm_mon == end->tm_mon &&
	    beg->tm_mday == end->tm_mday);
	print_date(same ? NULL : end);

	same = (beg->tm_hour == end->tm_hour && beg->tm_min == end->tm_min);
	print_time(same ? NULL : end);
}

static void
print_headerN(void)
{
	print_date(NULL);
	print_time(NULL);
}

static void
print_header(AgendaCtx *ctx, struct tm *beg, struct tm *end, size_t *num)
{
	switch ((*num)++) {
	case 0:
		print_header0(&ctx->beg, beg);
		break;
	case 1:
		print_header1(beg, end);
		break;
	default:
		print_headerN();
		break;
	}
}

static void
print(AgendaCtx *ctx, char **fields)
{
	struct tm beg = {0}, end = {0};
	time_t t;
	char const *e;

	t = strtonum(fields[FIELD_BEG], INT64_MIN, INT64_MAX, &e);
	if (e != NULL)
		err(1, "start time %s is %s", fields[FIELD_BEG], e);
	if (t > flag_to)
		return;
	localtime_r(&t, &beg);

	t = strtonum(fields[FIELD_END], INT64_MIN, INT64_MAX, &e);
	if (e != NULL)
		err(1, "end time %s is %s", fields[FIELD_END], e);
	if (t < flag_from)
		return;
	localtime_r(&t, &end);

	fputc('\n', stdout);
	for (size_t i = FIELD_OTHER, row = 0; i < ctx->fieldnum; i++) {
		if (*fields[i] == '\0')
			continue;
		print_header(ctx, &beg, &end, &row);
		fprintf(stdout, "%s\n", fields[i]);
	}

	ctx->beg = beg;
	ctx->end = end;
}

static void
tsv_to_agenda(AgendaCtx *ctx, FILE *fp)
{
	char *ln1 = NULL, *ln2 = NULL;
	size_t sz1 = 0, sz2 = 0;

	if (ctx->linenum == 0) {
		char *fields[FIELD_MAX];

		ctx->linenum++;
		getline(&ln1, &sz1, fp);
		if (ferror(fp))
			err(1, "reading stdin: %s", strerror(errno));
		if (feof(fp))
			err(1, "empty input");

		strchomp(ln1);
		ctx->fieldnum = strsplit(ln1, fields, FIELD_MAX, "\t");
		if (ctx->fieldnum == FIELD_MAX)
			err(1, "line 1: too many fields");
		if (ctx->fieldnum < FIELD_OTHER)
			err(1, "line 1: not enough input columns");
		if (strcasecmp(fields[0], "TYPE") != 0)
			err(1, "line 1: 1st column is not \"TYPE\"");
		if (strcasecmp(fields[1], "START") != 0)
			err(1, "line 1: 2nd column is not \"START\"");
		if (strcasecmp(fields[2], "END") != 0)
			err(1, "line 1: 3rd column is not \"END\"");
		if (strcasecmp(fields[3], "RECUR") != 0)
			err(1, "line 1: 4th column is not \"RECUR\"");
	}

	for (;;) {
		char *fields[FIELD_MAX];

		ctx->linenum++;
		getline(&ln2, &sz2, fp);
		if (ferror(fp))
			err(1, "reading stdin: %s", strerror(errno));
		if (feof(fp))
			break;

		strchomp(ln2);
		if (strsplit(ln2, fields, FIELD_MAX, "\t") != ctx->fieldnum)
			err(1, "line %zd: bad number of columns",
			    ctx->linenum, strerror(errno));

		print(ctx, fields);
	}
	fputc('\n', stdout);

	free(ln1);
	free(ln2);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-f fromdate] [-t todate]\n", arg0);
	exit(1);
}

int
main(int argc, char **argv)
{
	AgendaCtx ctx = {0};
	char c;

	if ((flag_from = time(NULL)) == (time_t)-1)
		err(1, "time: %s", strerror(errno));

	arg0 = *argv;
	while ((c = getopt(argc, argv, "f:t:")) > 0) {
		char const *e;

		switch (c) {
		case 'f':
			flag_from = strtonum(optarg, INT64_MIN, INT64_MAX, &e);
			if (e != NULL)
				err(1, "fromdate value %s is %s", optarg, e);
			break;
		case 't':
			flag_to = strtonum(optarg, INT64_MIN, INT64_MAX, &e);
			if (e != NULL)
				err(1, "todate value %s is %s", optarg, e);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (pledge("stdio", "") < 0)
		err(1, "pledge: %s", strerror(errno));

	tsv_to_agenda(&ctx, stdin);
	return 0;
}
