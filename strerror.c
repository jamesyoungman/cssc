/* strerror.c: Part of GNU CSSC.
 *
 *  Copyright (C) 1997 Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */
/* If we are to roll our own strerror(), we must have sys_errlist
 * and sys_nerr.
 */
#include <config.h>

#undef DIRE
#ifndef HAVE_SYS_ERRLIST
#define DIRE
#endif
#ifndef HAVE_SYS_NERR
#define DIRE
#endif


#if ERRNO_H_DECLARES_SYS_ERRLIST
#include <errno.h>
#elif STDLIB_H_DECLARES_SYS_ERRLIST
#include <stdlib.h>
#elif STDIO_H_DECLARES_SYS_ERRLIST
#include <stdio.h>
#else
extern const char *sys_errlist[];
#endif


#if ERRNO_H_DECLARES_SYS_NERR
#include <errno.h>
#elif STDLIB_H_DECLARES_SYS_NERR
#include <stdlib.h>
#elif STDIO_H_DECLARES_SYS_NERR
#include <stdio.h>
#else
extern int sys_nerr;
#endif

#ifdef DIRE
/*
 * No sys_nerr or no sys_errlist?
 * 
 * Wow, that's really dire!
 */
static const char *dunno = "don't know how to decode errors -- see" __FILE__;

char *
strerror(int errnum) 
{
  errnum = errnum; /* use the parameter to eliminate warning. */
  return (char*)dunno;
}

#else

static const char *unknown = "unknown error";

char *
strerror(int errnum)
{
  if (errnum < 0 || errnum >= sys_nerr)
    return (char*)unknown;
  else
    return (char*)sys_errlist[errnum];
}

#endif
