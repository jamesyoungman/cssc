/*
 * split.c: Part of GNU CSSC.
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
 * Defines the function split.
 *
 */
 
#include "cssc.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: split.cc,v 1.4 1997/07/02 18:05:54 james Exp $";
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
