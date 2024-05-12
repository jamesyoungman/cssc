/*
 * rel_list.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014, 2019,
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
 */
#include "config.h"
#include <algorithm>
#include <cstdlib>

#include "cssc.h"
#include "rel_list.h"
#include "ioerr.h"
#include "quit.h"

// Because we use member() all the time, this
// is a quadratic algorithm...but N is usually very small.
release_list::release_list(const char *s)
  : releases_()
{
  ASSERT(nullptr != s);

  char *p;
  while (*s)
    {
      long int n = strtol(s, &p, 10);
      if (p == s)
	break;			// not numeric.

      if (n < 0)
	ctor_fail(-1, "ranges not allowed in release lists");

      // add the entry if not already a member.
      const release r(n);
      if (!member(r))
	releases_.push_back(r);

      s = p;
      if (',' == *s)
	s++;
    }
}

// linear search for possible member.
bool release_list::member(release r) const
{
  return std::find(releases_.begin(), releases_.end(), r) != releases_.end();
}


release_list::release_list()
  : releases_()
{
}

release_list::release_list(const release_list &from)
  :releases_(from.releases_)
{
}

release_list::~release_list()
{
}

cssc::Failure release_list::print(FILE * out) const
{
  bool first = true;
  for (const auto& release : releases_)
    {
      if (!first)
	{
	  if (putc_failed(fputc(' ', out)))
	    return cssc::make_failure_from_errno(errno);
	}
      first = false;
      auto printed = release.print(out);
      if (!printed.ok())
	return printed;
    }
  return cssc::Failure::Ok();
}


/* Local variables: */
/* mode: c++ */
/* End: */
