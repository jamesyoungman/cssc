/*
 * prompt.cc: Part of GNU CSSC.
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
static const char rcs_id[] = "CSSC $Id: prompt.cc,v 1.12 2007/06/19 23:14:45 james_youngman Exp $";
#endif


static char *
re_new(char *p, int oldlen, int newlen)
{
  char *q = new char[newlen];
  if (q)
    {
      memcpy(q, p, oldlen);
    }
  delete[] p;
  return q;
}



/* Prompts the user for input. */

mystring
prompt_user(const char *prompt)
{
  const int chunk_size = 4;	// TODO: Debug the code, then increase this!
  char *linebuf = new char [chunk_size];
  int buflen = chunk_size;
  int c, lastc;
  int i = 0;
  
  // Issue the prompt only if stdin is a tty.
  if (stdin_is_a_tty())
    {
      fputs(prompt, stdout);
      fflush(stdout);
    }

  lastc = 0;
  while ( (c=getchar()) != EOF )
    {
      if ('\n' == c)
	{
	  if (lastc == '\\')
	    --i; /* Overwrite the backslash with the escaped newline */
	  else
	    break;
	}
      
      if (i == buflen - 1)
	{
	  linebuf = re_new(linebuf, buflen, buflen+chunk_size);
	  buflen += chunk_size;
	}
      linebuf[i++] = c;
      lastc = c;
    }

  linebuf[i] = '\0';
  mystring ret(linebuf);
  delete[] linebuf;
  
  return ret;
}

/* Local variables: */
/* mode: c++ */
/* End: */
