/*
 * delta-iterator.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999 Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of class delta_iterator.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-iterator.h"
#include "delta-table.h"


#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: delta-iterator.cc,v 1.3 1999/03/19 23:58:34 james Exp $";
#endif

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
const_delta_iterator::next(int all = 0)
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
