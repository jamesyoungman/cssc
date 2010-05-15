/*
 * quit.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1999,2001,2007 Free Software Foundation, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 */

#ifndef CSSC__QUIT_H__
#define CSSC__QUIT_H__

#include <stdarg.h>

#include "cssc-assert.h"

#include "cleanup.h"

extern const char *prg_name;

void set_prg_name(const char *name);

// errormsg(): emit an error message preceded by the program name.
//             then return to the caller (don't exit).
void errormsg(const char *fmt, ...);
void warning (const char *fmt, ...);
// void v_errormsg(const char *fmt, va_list ap);

void v_unknown_feature_warning(const char *fmt, va_list ap);

// errormsg_with_errno(): emits
//   prog: your-text-here: errno-error-message \n
// to STDERR.
void errormsg_with_errno(const char *fmt, ...);


void quit_on_fatal_signals(void); // defined in fatalsig.cc.




#ifdef __GNUC__
NORETURN fatal_quit(int err, const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) POSTDECL_NORETURN;
NORETURN p_corrupt_quit(const char *fmt, ...)
        __attribute__((format(printf, 1, 2))) POSTDECL_NORETURN;
NORETURN s_corrupt_quit(const char *fmt, ...)
        __attribute__((format(printf, 1, 2))) POSTDECL_NORETURN;
NORETURN s_missing_quit(const char *fmt, ...)
        __attribute__((format(printf, 1, 2))) POSTDECL_NORETURN;
NORETURN ctor_fail(int err, const char *fmt, ...)
        __attribute__((format(printf, 2, 3))) POSTDECL_NORETURN;
#else
NORETURN s_missing_quit(const char *fmt, ...) POSTDECL_NORETURN;
NORETURN s_corrupt_quit(const char *fmt, ...)  POSTDECL_NORETURN;
NORETURN p_corrupt_quit(const char *fmt, ...)  POSTDECL_NORETURN;
NORETURN fatal_quit(int err, const char *fmt, ...)  POSTDECL_NORETURN;
NORETURN ctor_fail(int err, const char *fmt, ...)  POSTDECL_NORETURN;
#endif

NORETURN s_unrecognised_feature_quit(const char *fmt, va_list ap) POSTDECL_NORETURN;
NORETURN ctor_fail_nomsg(int err)  POSTDECL_NORETURN;
NORETURN nomem()  POSTDECL_NORETURN;
extern void usage();

#endif /* __QUIT_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
