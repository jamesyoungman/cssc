/*
 * sf-kw.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998, Free Software Foundation, Inc. 
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
 *
 * sccs_file::no_id_keywords()
 */

#ifdef __GNUC__
#pragma implementation "seqstate.h"
#endif

#include "cssc.h"
#include "sccsfile.h"
#include "except.h"


void sccs_file::
no_id_keywords(const char name[]) const 
{
  if (flags.no_id_keywords_is_fatal)
    {
      // TODO: Just what does "fatal" mean for no_id_keywords_is_fatal ?
#ifdef HAVE_EXCEPTIONS
      errormsg("%s: Warning: No id keywords.", name);
      throw CsscNoKeywordsException();
#else
      fatal_quit(-1, "%s: Warning: No id keywords.", name);
#endif      
    }
  else
    {
      errormsg("%s: Warning: No id keywords.", name);
    }
}

/* Warns or quits if the new delta doesn't include any id keywords */

bool
sccs_file::check_keywords_in_file(const char *name)
{
  FILE *f = fopen(name, "r");
  if (NULL == f)
    {
      errormsg_with_errno("%s: Can't open file for reading", name);
      return false;
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
		  return true;
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
  return true;
}


