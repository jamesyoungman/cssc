/*
 * l-split.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 * Functions for non-destructively spliting a string into a list of
 * strings.
 * 
 */

#include "cssc.h"
#include "mylist.h"

#include <string.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: l-split.cc,v 1.9 1998/02/21 14:27:08 james Exp $";
#endif

list<mystring>
split_mrs(mystring mrs)
{
  list<mystring> mr_list;
  const char *delims = " \t\n";
  
  if (!mrs.empty())
    {
      char *s = xstrdup(mrs.c_str());
      char *p = strtok(s, delims);
      
      while(p)
	{
	  mr_list.add(p);
	  p = strtok(NULL, delims);
	}
      free(s);
    }

  return mr_list;
}

list<mystring>
split_comments(mystring comments) {
	list<mystring> comment_list;

	if (!comments.empty()) {
		char *s = xstrdup(comments.c_str());
		char *start;
		char *end;

		start = s;
		end = strchr(s, '\n');
		while(end != NULL) {
			*end++ = '\0';
			comment_list.add(start);
			start = end;
			end = strchr(start, '\n');
		}

		if (*start != '\0') {
			comment_list.add(start);
		}

		free(s);
	}

	return comment_list;
}

/* Local variables: */
/* mode: c++ */
/* End: */
