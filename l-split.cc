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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
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
static const char rcs_id[] = "CSSC $Id: l-split.cc,v 1.13 2001/09/29 19:39:41 james_youngman Exp $";
#endif

mylist<mystring>
split_mrs(mystring mrs)
{
  mylist<mystring> mr_list;
  const char *delims = " \t\n";
  
  if (!mrs.empty())
    {
      char *s = new char[strlen(mrs.c_str()) + 1];
      memcpy( s, mrs.c_str(), strlen(mrs.c_str()) + 1);
      char *p = strtok(s, delims);
      
      while (p)
	{
	  mr_list.add(p);
	  p = strtok(NULL, delims);
	}
      delete[] s;
    }

  return mr_list;
}

mylist<mystring>
split_comments(mystring comments) {
	mylist<mystring> comment_list;

	if (!comments.empty()) {
	  char *s = new char[strlen(comments.c_str()) + 1];
	  memcpy( s, comments.c_str(), strlen(comments.c_str()) + 1);
		char *start;
		char *end;

		start = s;
		end = strchr(s, '\n');
		while (end != NULL) {
			*end++ = '\0';
			comment_list.add(start);
			start = end;
			end = strchr(start, '\n');
		}

		if (*start != '\0') {
			comment_list.add(start);
		}

		delete[] s;
	}

	return comment_list;
}

/* Local variables: */
/* mode: c++ */
/* End: */
