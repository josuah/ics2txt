#ifndef ICAL_H
#define ICAL_H

#include <stdio.h>
#include <time.h>

#define ICAL_STACK_SIZE 10

typedef struct IcalParser IcalParser;
typedef struct IcalStack IcalStack;

struct IcalStack {
	char	 name[32];
	char	 tzid[32];
};

struct IcalParser {
	/* function called while parsing in this order */
	int (*fn_entry_name)(IcalParser *, char *);
	int (*fn_param_name)(IcalParser *, char *);
	int (*fn_param_value)(IcalParser *, char *, char *);
	int (*fn_entry_value)(IcalParser *, char *, char *);
	int (*fn_block_begin)(IcalParser *, char *);
	int (*fn_block_end)(IcalParser *, char *);
	/* if returning non-zero then halt the parser */

	int	 base64;
	char const *errmsg;
	size_t	 linenum;
	char	*tzid;

	IcalStack stack[ICAL_STACK_SIZE], *current;
};

int	ical_parse(IcalParser *, FILE *);
int	ical_get_level(IcalParser *);
int	ical_get_time(IcalParser *, char *, time_t *);
int	ical_get_value(IcalParser *, char *, size_t *);
int	ical_error(IcalParser *, char const *);

#endif
