/*
 * mysc.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Master include file for MySC.
 *
 * @(#) MySC mysc.h 1.3 93/12/30 19:48:09
 *
 */

#ifndef __MYSC_H__
#define __MYSC_H__

#undef TESTING

#include "config.h"
#include "defaults.h"

#ifdef CONFIG_INCLUDE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#ifdef CONFIG_DECLARE_ERRNO
extern int errno;
#endif

#ifndef NO_COMMON_HEADERS

#include "quit.h"
#include "xalloc.h"
#include "mystring.h"
#include "file.h"

typedef unsigned short seq_no;

int split(char *s, char **args, int len, char c);
string prompt_user(char const *prompt);

#endif /* NO_COMMON_HEADERS */

extern char const main_sccs_id[];

inline void
version() {
	fprintf(stderr, "%s\n@(#) MySC Release: 2; Patchlevel: 0\n",
		main_sccs_id);
}

#endif /* __MYSC_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
