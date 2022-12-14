#ifndef ICAL_H
#define ICAL_H

#include <stdio.h>
#include <time.h>

#define ICAL_STACK_SIZE 10

typedef enum {
	ICAL_BLOCK_VEVENT,
	ICAL_BLOCK_VTODO,
	ICAL_BLOCK_VJOURNAL,
	ICAL_BLOCK_VFREEBUSY,
	ICAL_BLOCK_VALARM,
	ICAL_BLOCK_OTHER,
} IcalBlock;

typedef struct {
	char	 name[32];
	char	 tzid[32];
} IcalStack;

typedef struct IcalParser IcalParser;
struct IcalParser {
	/* function called while parsing in this order */
	int (*fn_field_name)(IcalParser *, char *);
	int (*fn_param_name)(IcalParser *, char *);
	int (*fn_param_value)(IcalParser *, char *, char *);
	int (*fn_field_value)(IcalParser *, char *, char *);
	int (*fn_block_begin)(IcalParser *, char *);
	int (*fn_block_end)(IcalParser *, char *);
	/* if returning non-zero then halt the parser */

	int	 base64;
	char	*errmsg;
	size_t	 linenum;
	char	*tzid;
	IcalBlock blocktype;
	IcalStack stack[ICAL_STACK_SIZE], *current;
};

extern char *ical_block_name[ICAL_BLOCK_OTHER + 1];

int	 ical_parse(IcalParser *, FILE *);
int	 ical_get_level(IcalParser *);
int	 ical_get_time(IcalParser *, char *, time_t *);
int	 ical_get_value(IcalParser *, char *, size_t *);
int	 ical_err(IcalParser *, char *);

#endif
