/*
 * sid_list.c: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Test code (only!) for sid_list.h.
 *
 * @(#) CSSC sid_list.c 1.1 93/11/09 17:18:03
 *
 */


#ifdef TEST
#include <stdarg.h>
#include <stdlib.h>
#include "cssc.h"
#include "mylist.h"
#include "linebuf.h"
#include "sid.h"
#include "xalloc.cc"
#include "linebuf.cc"
#include "sid.cc"

// TEST - only.
NORETURN
quit(int err, const char *fmt, ...)
{
  va_list ap;
  ap = va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  exit(abs(err));
}
NORETURN nomem() { quit(-4, "Out of memory!"); }
NORETURN assert_failed(char const *file, int line, char const *expr)
{
  quit(-1, "assertion \"%s\" failed at %s, line %d\n", expr, file, line);
}


#endif
		
/* Local variables: */
/* mode: c++ */
/* End: */
