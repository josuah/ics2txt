#ifndef ICAL_H
#define ICAL_H

#include <stdio.h>
#include <time.h>

typedef struct IcalParser IcalParser;
struct IcalParser {
	/* function called on content */
	int (*fn_entry_name)(IcalParser *, char *);
	int (*fn_param_name)(IcalParser *, char *);
	int (*fn_param_value)(IcalParser *, char *, char *);
	int (*fn_entry_value)(IcalParser *, char *, char *);
	int (*fn_block_begin)(IcalParser *, char *);
	int (*fn_block_end)(IcalParser *, char *);
	/* if returning non-zero then halt the parser */

	int	 base64encoded;
	char const *errmsg;
	size_t	 line;

	/* stack of blocks names: "name1\0name2\0...nameN\0\0" */
	int	 level;
	char	 stack[1024];
};

int	 ical_parse(IcalParser *, FILE *);
//TODO: char	*ical_get_time(char *);
//TODO: char	*ical_get_value(IcalCtx *, char *);

#endif
