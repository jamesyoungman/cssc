/*
 * strstr.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * An implementation of the ANSI C library function strstr.
 *
 * @(#) MySC strstr.c 1.1 93/11/09 17:18:03
 *
 */

#ifndef __STRSTR_C__
#define __STRSTR_C__

#include "mysc.h"

#ifndef HAVE_STRSTR
#define strstr LIDENT(strstr)

static char *
strstr(const char *s1, const char *s2) {
	char c = *s2;

	if (c == '\0') {
		return (char *) s1; /* NULL? */
	}

	s1 = strchr(s1, c);
	while(s1 != NULL && strcmp(s1 + 1, s2 + 1) != 0) {
		s1 = strchr(s1 + 1, c);
	}

	return (char *) s1;
}
#endif /*  HAVE_STRSTR */
#endif /* __STRSTR_C__ */
