/*
 * dtbl-prepend.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1999,2007, 2008, 2009, 2010, 2011, 2014 Free Software Foundation, Inc.
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

#include <config.h>

#include "cssc.h"
#include "delta-table.h"


/* Insert a delta at the start of the delta table. */

void
cssc_delta_table::prepend(const delta &it)
{
  delta_list newlist;

  newlist.add(it);
  newlist += l;

  l = newlist;
}

/* Local variables: */
/* mode: c++ */
/* End: */
