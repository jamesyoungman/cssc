/*
 * rl-merge.cc: Part of GNU CSSC.
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 */

#include "cssc.h"
#include "rel_list.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: rl-merge.cc,v 1.6 2007/06/19 23:14:46 james_youngman Exp $";
#endif

// another horrendously inefficient implementation.
void release_list::merge(const release_list& m)
{
  const int mlen = m.l.length();
  for(int i=0; i<mlen; i++)
    {
      const release r = m.l[i];
      if (!member(r))
	l.add(r);
    }
}

// another horrendously inefficient implementation.
void release_list::remove(const release_list& rm)
{
  const int len = l.length();
  mylist<release> newlist;
  
  for(int i=0; i<len; i++)
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
