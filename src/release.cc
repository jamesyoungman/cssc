/*
 * release.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014, 2019
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Members of the class release.
 */
#include <config.h>

#include "release.h"

#include "sid.h"

release::release(const sid &s)
  :  rel_(s.get_release())
{
  // nothing.
}

release::release(const char *s)
  : release()
{
  if (s == nullptr)
    {
      // TODO: the default constructor sets rel_=-1.
      // Is there a meaningful difference here?
      rel_ = 0;
      return;
    }

  rel_ = get_id_comp(s);

  if (*s != '\0' || rel_ == 0)
    {
      rel_ = -1;
    }
}

