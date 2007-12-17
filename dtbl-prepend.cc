/*
 * dtbl-prepend.cc: Part of GNU CSSC.
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
 * 
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta-table.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: dtbl-prepend.cc,v 1.7 2007/12/17 21:59:48 jay Exp $";
#endif


/* Insert a delta at the start of the delta table. */

void
cssc_delta_table::prepend(const delta &it)
{
  mylist<struct delta> newlist;

  newlist.add(it);
  newlist += l;

  l = newlist;
  
  update_highest(it);
}


// Explicit template instantiations.
template mylist<delta>& operator+=<delta>(mylist<delta>&, mylist<delta> const&);

/* Local variables: */
/* mode: c++ */
/* End: */
