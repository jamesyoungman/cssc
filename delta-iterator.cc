/*
 * delta-iterator.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2007 Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of class delta_iterator.
 *
 */

#include "delta-iterator.h"
#include "delta-table.h"
#include "cssc-assert.h"

delta_iterator::delta_iterator(cssc_delta_table *d)
  : dtbl(d), pos(-1)
{
}

int
delta_iterator::next(int all)
{
  ASSERT(0 != dtbl);
  
  while (++pos < dtbl->length())
    {
      if (all || !dtbl->select(pos).removed())
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
  return &dtbl->select(pos);
}

delta *
delta_iterator::operator ->()
{
  ASSERT(0 != dtbl);
  return &dtbl->select(pos);
}


////////////////////////////////////////////////////////////


const_delta_iterator::const_delta_iterator(cssc_delta_table const *d)
  : dtbl(d), pos(-1)
{
}

int
const_delta_iterator::next(int all)
{
  ASSERT(0 != dtbl);
  while (++pos < dtbl->length())
    {
      if (all || !dtbl->select(pos).removed())
	{
	  return 1;
	}
    }
  return 0;
}
  
int const_delta_iterator::index() const
{
  return pos;
}

delta const *
const_delta_iterator::operator ->() const
{
  ASSERT(0 != dtbl);
  return &dtbl->select(pos);
}



/* Local variables: */
/* mode: c++ */
/* End: */
