/*
 * prompt.c: Part of GNU CSSC.
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
 * Defines the function prompt_user.
 *
 */

#include "cssc.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: prompt.cc,v 1.5 1997/07/02 18:18:38 james Exp $";
#endif

/* Prompts the user for input. */

mystring
prompt_user(const char *prompt) {
	static char *linebuf = (char *) xmalloc(CONFIG_LINEBUF_CHUNK_SIZE);
	static int buflen = CONFIG_LINEBUF_CHUNK_SIZE;
	int c;
	int i = 0;

	fputs(prompt, stdout);
	fflush(stdout);
	
	c = getchar();
	while(c != EOF && c != '\n') {
		if (i == buflen - 1) {
			buflen += CONFIG_LINEBUF_CHUNK_SIZE;
			linebuf = (char *) xrealloc(linebuf, buflen);
		}
		linebuf[i++] = c;
		c = getchar();
	}

	linebuf[i] = '\0';
	return linebuf;
}

/* Local variables: */
/* mode: c++ */
/* End: */
