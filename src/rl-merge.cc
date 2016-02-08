/*
 * rl-merge.cc: Part of GNU CSSC.
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
 */

#include <config.h>
#include "cssc.h"
#include "rel_list.h"

// another horrendously inefficient implementation.
void release_list::merge(const release_list& m)
{
  for (const auto& r : m)
    {
      if (!member(r))
	l.add(r);
    }
}

// another horrendously inefficient implementation.
void release_list::remove(const release_list& rm)
{
  mylist<release> newlist;

  for (const auto& r : l)
    {
      if (!rm.member(r))
	newlist.add(r);
    }
  l = newlist;
}



/* Local variables: */
/* mode: c++ */
/* End: */
