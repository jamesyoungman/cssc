/*
 * sf-chkid.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019,
 *  2024 Free Software Foundation, Inc.
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 */

#include <config.h>

#include <string.h>

#include "cssc.h"
#include "sccsfile.h"
#include "linebuf.h"
#include "bodyio.h"

bool
is_id_keyword_letter(char ch)
{
  return strchr("MIRLBSDHTEGUYFPQCZWA", ch) != NULL;
}


/* check_id_keywords():
 *
 * Returns true if the string contains a valid SCCS id keyword.  This
 * version is 8-bit-clean.  We use memchr() if available.  No harm
 * done if it is not.
 *
 * memchr() is probably not a great deal faster than the while loop we
 * use below, so it's not worth bothering with it inside the loop.
 * However, it provides a quick way of eliminating the majority of
 * cases.
 *
 * If we know s[2] is a % then s[1] cannot be the start of a keyword,
 * because "%%%" is not a valid keyword.  This optimisation probably
 * isn't worth the extra opaqueness of the resulting code.  It might
 * even be slower.  So we don't use it.
 *
 * We need three characters to contain an ID.  subtracting two at the
 * start allows us to test against (>) zero for the loop.
 */
bool
check_id_keywords(const char *s, size_t len)
{
  if (len < 3)
    return false;		// anything shorter cannot contain an ID.

  const void *pv = memchr(s, '%', len);
  if (nullptr == pv)
    {
      return false;		// no %, hence no keywords.
    }
  else			// skip forward to first percent sign.
    {
      const char *pc = static_cast<const char *>(pv);
      len -= (pc - s);
      s = pc;

      // Having adjusted len, we need to retest it.
      if (len < 3)
	return false;
    }

  len -= 2u;

  while (len-- > 0)
    {
      // test the % characters first to avoid some unneccesary function calls.
      if ('%' == s[0] && s[1] && '%' == s[2] && is_id_keyword_letter(s[1]))
	return true;
      ++s;
    }
  return false;
}

/* Local variables: */
/* mode: c++ */
/* End: */
