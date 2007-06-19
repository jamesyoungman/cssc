/*
 * list.cc: Part of GNU CSSC.
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Code for the template mylist<T>.
 */

mylist<const char*> &
operator +=(list<const char*> &l1, list<mystring> const &l2)
{
  int len = l2.length();
  int i;
  for(i = 0; i < len; i++)
    {
      // This add operation would be push_back() under STL.
      // When everybody supports STL, we'll switch.
      l1.add(l2[i].c_str());
    }
  return l1;
}

/* Local variables: */
/* mode: c++ */
/* End: */
