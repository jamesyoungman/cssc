/*
 * split.c
 * 
 * By Ross Ridge
 * Public Domain
 *
 * Defines the function split.
 *
 */
 
#include "mysc.h"

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC split.c 1.1 93/11/09 17:18:03";
#endif

/* Destructively spilts a string into an argument list. */

int
split(char *s, char **args, int len, char c) {
	char *start = s;
	char *end = strchr(start, c);
	int i;

	for(i = 0; i < len; i++) {
		args[i] = start;
		if (end	== NULL) {
			if (start[0] != '\0') {
				i++;
			}
			return i;
		}
		*end++ = '\0';
		start = end;
		end = strchr(start, c);
	}

	return i;
}

/* Local variables: */
/* mode: c++ */
/* End: */
