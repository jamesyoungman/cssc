/*
 * sf-kw.cc: Part of GNU CSSC.
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
 *
 * sccs_file::no_id_keywords()
 */

#ifdef __GNUC__
#pragma implementation "seqstate.h"
#endif

#include "cssc.h"
#include "sccsfile.h"



void sccs_file::
no_id_keywords(const char name[]) const 
{
  if (flags.no_id_keywords_is_fatal)
    {
      quit(-1, "%s: No id keywords.", name);
    }
  else
    {
      fprintf(stderr, "%s: Warning: No id keywords.\n", name);
    }
}

/* Warns or quits if the new delta doesn't include any id keywords */

void
sccs_file::check_keywords_in_file(const char *name)
{
  FILE *f = fopen(name, "r");
  if (NULL == f)
    {
      quit(errno, "%s: Can't open file for reading.", name);
    }
  else
    {
      int ch, last;
      
      last = '\n';
      while ( EOF != (ch=getc(f)) )
	{
	  if ('%' == last && is_id_keyword_letter(ch))
	    {
	      const int peek = getc(f);
	      if ('%' == peek)
		{
		  fclose(f);
		  return;
		}
	      else if (EOF == peek)
		{
		  break;
		}
	      ungetc(peek, f);
	    }
	  last = ch;
	}
      no_id_keywords(name);
    }
  fclose(f);
}


