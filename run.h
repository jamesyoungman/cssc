/*
 * run.h: Part of GNU CSSC.
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
 * @(#) CSSC run.h 1.1 93/11/09 17:17:49
 *
 */

#ifndef __RUN_H__
#define __RUN_H__

#include "list.h"

#ifndef HAVE_FORK

#if !defined(HAVE_SPAWN) && !defined(CONFIG_DJGPP)

#define STATUS(n) (0)
#define STATUS_MSG(n) 

#else /* !defined(HAVE_SPAWN) && !defined(CONFIG_DJGPP) */

#define STATUS(n) (n)
#define STATUS_MSG(n) "(status = %d)", (n)

#endif /* !defined(HAVE_SPAWN) && !defined(CONFIG_DJGPP) */

#else /* HAVE_FORK is defined */

#define STATUS(n) ((n) << 8)
#define STATUS_MSG(n) "(status = %d, %d)", (n) >> 8, (n) & 0xff

#endif /* HAVE_FORK */

int run(const char *prg, list<const char *> const &args);
int run_mr_checker(const char *prg, const char *arg1,
		   list<mystring> mrs);

#endif /* __RUN_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
