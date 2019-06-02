/*
 * sf-kw.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019
 *  Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * sccs_file::no_id_keywords()
 */

#include <config.h>

#include "cssc.h"
#include "cleanup.h"
#include "failure.h"
#include "sccsfile.h"
#include "except.h"
#include "file.h"


void sccs_file::no_id_keywords(const char filename[]) const
{
  const std::string msg = std::string(filename)
    + ": No id keywords";
  warning("%s", msg.c_str());
  if (flags.no_id_keywords_is_fatal)
    {
      // TODO: Just what does "fatal" mean for no_id_keywords_is_fatal ?
      throw CsscNoKeywordsException();
    }
}

/* Warns or quits if the new delta doesn't include any id keywords */

cssc::Failure
sccs_file::check_keywords_in_file(const char *filename)
{
  FILE *fp = fopen_as_real_user(filename, "r");
  if (NULL == fp)
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "failed to open " << filename << " for reading";
    }
  ResourceCleanup closer([fp](){ fclose(fp); });
  int ch, last;

  last = '\n';
  errno = 0;
  while ( EOF != (ch=getc(fp)) )
    {
      if ('%' == last && is_id_keyword_letter(ch))
	{
	  const int peek = getc(fp);
	  if (EOF == peek)
	    break;
	  if ('%' == peek)
	    {
	      // We found an id keyword.
	      return cssc::Failure::Ok();
	    }
	  // TODO: the ungetc() below may be unnecessary; we know that
	  // ch is a keyword letter, so it cannot be '%'.  If peek is
	  // not '%', then we saw '%Wz' or something like it.  There
	  // is probably no need to unget the 'z' since it cannot
	  // start an id keyword.
	  if (EOF == ungetc(peek, fp))
	    break;
	}
      last = ch;
    }
  const int saved_errno = ferror(fp) ? errno : 0;
  // Issue the warning or fatal error.  If it's a warning, this
  // function needs to return successfully so that the delta
  // operation continues.
  no_id_keywords(filename);
  if (saved_errno)
    {
      return cssc::make_failure_builder_from_errno(saved_errno)
	<< "read error on " << filename;
    }
  return cssc::Failure::Ok();
}
