/*
 * dtbl-prepend.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 * 
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta-table.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: dtbl-prepend.cc,v 1.2 1999/04/18 17:39:40 james Exp $";
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


