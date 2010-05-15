/*
 * sf-kw.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1998,2007 Free Software Foundation, Inc.
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
 *
 * sccs_file::no_id_keywords()
 */

#include "cssc.h"
#include "sccsfile.h"
#include "except.h"
#include "file.h"


void sccs_file::
no_id_keywords(const char filename[]) const
{
  if (flags.no_id_keywords_is_fatal)
    {
      // TODO: Just what does "fatal" mean for no_id_keywords_is_fatal ?
      warning("%s: No id keywords.", filename);
      throw CsscNoKeywordsException();
    }
  else
    {
      warning("%s: No id keywords.", filename);
    }
}

/* Warns or quits if the new delta doesn't include any id keywords */

bool
sccs_file::check_keywords_in_file(const char *filename)
{
  FILE *fp = fopen_as_real_user(filename, "r");
  if (NULL == fp)
    {
      errormsg_with_errno("%s: Can't open file for reading", filename);
      return false;
    }
  else
    {
      int ch, last;

      last = '\n';
      while ( EOF != (ch=getc(fp)) )
	{
	  if ('%' == last && is_id_keyword_letter(ch))
	    {
	      const int peek = getc(fp);
	      if ('%' == peek)
		{
		  fclose(fp);
		  return true;
		}
	      else if (EOF == peek)
		{
		  break;
		}
	      ungetc(peek, fp);
	    }
	  last = ch;
	}
      no_id_keywords(filename);
    }
  fclose(fp);
  return true;
}
