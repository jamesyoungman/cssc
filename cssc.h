/*
 * cssc.h
 *
 * Master include file for CSSC.
 *
 * $Id: cssc.h,v 1.6 1997/11/06 22:37:02 james Exp $
 *
 */

#ifndef CSSC__CSSC_H__
#define CSSC__CSSC_H__

#undef TESTING

#include "config.h"
#include "defaults.h"

#ifdef HAVE_SYS_TYPES_H
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
mystring prompt_user(const char *prompt);

#endif /* NO_COMMON_HEADERS */


#endif

/* Local variables: */
/* mode: c++ */
/* End: */
