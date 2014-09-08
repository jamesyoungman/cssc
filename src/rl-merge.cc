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
  const mylist<release>::size_type mlen(m.l.length());
  for(mylist<release>::size_type i=0; i<mlen; i++)
    {
      const release r = m.l[i];
      if (!member(r))
	l.add(r);
    }
}

// another horrendously inefficient implementation.
void release_list::remove(const release_list& rm)
{
  const mylist<release>::size_type len(l.length());
  mylist<release> newlist;

  for(mylist<release>::size_type i=0; i<len; i++)
    {
      const release r = l[i];
      if (!rm.member(r))
	newlist.add(r);
    }
  l = newlist;
}



/* Local variables: */
/* mode: c++ */
/* End: */
