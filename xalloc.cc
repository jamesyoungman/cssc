/*
 * xalloc.c: Part of GNU CSSC.
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
 * Interface to the standard memory allocation routines that quit if
 * there isn't enough memory available. 
 *
 */

#include "cssc.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: xalloc.cc,v 1.6 1998/02/28 14:49:43 james Exp $";
#endif

#ifdef CONFIG_DECLARE_MALLOC
extern "C" {
	void * CDECL malloc(size_t);
	void * CDECL realloc(void *, size_t);
	void * CDECL calloc(size_t, size_t);
};
#endif


/* Local variables: */
/* mode: c++ */
/* End: */
