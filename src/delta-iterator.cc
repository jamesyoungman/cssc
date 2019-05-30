/*
 * delta-iterator.cc: Part of GNU CSSC.
 *
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
 * Members of class delta_iterator.
 *
 */

#include <config.h>

#include "cssc.h"
#include "delta-iterator.h"
#include "delta-table.h"
#include "cssc-assert.h"

delta_iterator::delta_iterator(cssc_delta_table *d, delta_selector selector)
  : dtbl(d), pos(-1), selector_(selector)
{
}

bool
delta_iterator::next()
{
  ASSERT(0 != dtbl);

  while (++pos < dtbl->size())
    {
      if (all() || !dtbl->at(pos).removed())
	{
	  return 1;
	}
    }
  return 0;
}

int delta_iterator::index() const
{
  return pos;
}

delta const *
delta_iterator::operator ->() const
{
  ASSERT(0 != dtbl);
  return &dtbl->at(pos);
}

delta *
delta_iterator::operator ->()
{
  ASSERT(0 != dtbl);
  return &dtbl->at(pos);
}


////////////////////////////////////////////////////////////


const_delta_iterator::const_delta_iterator(cssc_delta_table const *d, delta_selector selector)
  : dtbl(d), pos(-1), selector_(selector)
{
}

bool
const_delta_iterator::next()
{
  ASSERT(0 != dtbl);
  while (++pos < dtbl->size())
    {
      if (all() || !dtbl->at(pos).removed())
	{
	  return true;
	}
    }
  return false;
}

int const_delta_iterator::index() const
{
  return pos;
}

delta const *
const_delta_iterator::operator ->() const
{
  ASSERT(0 != dtbl);
  return &dtbl->at(pos);
}



/* Local variables: */
/* mode: c++ */
/* End: */
