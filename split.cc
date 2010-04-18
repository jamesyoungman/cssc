/*
 * split.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc. 
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
 *
 * Defines the function split.
 *
 */
 
#include "cssc.h"

/* Destructively spilts a string into an argument list. */

#if 0
int
split(char *s, char **args, int len, char c)
{
  char *start = s;
  char *end = strchr(start, c);
  int i;

  for(i = 0; i < len; i++)
    {
      args[i] = start;
      if (0 == end)
	{
	  if (start[0] != '\0')
	    i++;
	  else
	    return i;		// no more delimiters.
	}
      *end++ = '\0';
      start = end;
      end = strchr(start, c);
    }

  return i;
}
#endif

/* Local variables: */
/* mode: c++ */
/* End: */
