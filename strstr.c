/*
 * strstr.c: Part of GNU CSSC.
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
 * An implementation of the ANSI C library function strstr.
 *
 *
 */

#include <config.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

char *
strstr(const char *s1, const char *s2) {
	char c = *s2;

	if (c == '\0') {
		return (char *) s1; /* NULL? */
	}

	s1 = strchr(s1, c);
	while(s1  && strcmp(s1 + 1, s2 + 1) != 0) {
		s1 = strchr(s1 + 1, c);
	}

	return (char *) s1;
}
