/*
 * getopt.c
 *
 * By Ross Ridge
 * Public Domain
 * 
 * Members of the class getopt.
 *
 */

#ifdef __GNUC__
#pragma implementation "getopt.h"
#endif

#include "mysc.h"
#include "getopt.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC getopt.c 1.1 93/11/09 17:17:54";
#endif

int
getopt::next() {
	if (cindex == NULL || *cindex == '\0') {
		if (index >= argc) {
			return END_OF_ARGUMENTS;
		}
		cindex = argv[index];
		if (cindex[0] != '-' || cindex[1] == '\0') {
			return END_OF_ARGUMENTS;
		}
		index++;
		if (cindex[1] == '-' && cindex[2] == '\0') {
			return END_OF_ARGUMENTS;
		}
		cindex++;
	}

	char c = *cindex++;
	char *match = strchr(opts, c);
	
	if (c == '\0' || match == NULL) {
		if (opterr) {
			quit(-2, "Unrecognized option '%c'.\n", c);
		}
		arg = cindex - 1;
		return UNRECOGNIZED_OPTION;
	}

	if (match[1] == ':') {
		if (*cindex == '\0') {
			if (index >= argc) {
				if (opterr) {
					quit(-2, "Option '%c' requires"
						 "an argument.\n", c);
				}
				arg = cindex - 1;
				return MISSING_ARGUMENT;
			}
			arg = argv[index++];
		} else {
			arg = cindex;
		}
		cindex = NULL;
	} else {
		arg = NULL;
	}

	return c;
}

/* Local variables: */
/* mode: c++ */
/* End: */
