/*
 * release.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 * Declaration of class release
 *
 */


#ifndef INC_RELEASE_H
#define INC_RELEASE_H

#include <cstdio>
#include "cssc-assert.h"
#include "failure.h"

class sid;

class release
{
  short rel;

  //  release(short r, sid const *): rel(r) {}

public:
  release(): rel(-1) {}
  release(short int r): rel(r) {}
  release(const char *s);
  release(const sid& s);

  bool valid() const { return rel > 0; }

  enum { LARGEST = 9999 };	// largest valid release number.

  release &operator++() { rel++; return *this; }
  release &operator--() { rel--; return *this; }
  operator unsigned long() const
  {
    ASSERT(valid());
    return static_cast<unsigned long>(rel);
  }

  operator short() const
  {
    return rel;
  }

  friend bool operator <(release r1, release r2)
    {
      return r1.rel < r2.rel;
    }

  friend bool operator >(release r1, release r2)
    {
      return r1.rel > r2.rel;
    }

  friend bool operator <=(release r1, release r2)
    {
      return r1.rel <= r2.rel;
    }

  friend bool operator >=(release r1, release r2)
    {
      return r1.rel >= r2.rel;
    }

  friend bool operator ==(release r1, release r2)
    {
      return r1.rel == r2.rel;
    }

  friend bool operator !=(release r1, release r2)
    {
      return r1.rel != r2.rel;
    }

  cssc::Failure print(FILE *out) const {
    if (fprintf(out, "%d", rel) < 0)
      {
	return cssc::make_failure_from_errno(errno);
      }
    return cssc::Failure::Ok();
  }
};

/* Local variables: */
/* mode: c++ */
/* End: */
#endif
