/*
 * my-algo.h: Part of GNU CSSC.
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
 * Definition of additional generic algorithms.
 *
 */
 


#ifndef CSSC_MY_ALGO_H
#define CSSC_MY_ALGO_H

#ifndef ASSERT
#error need ASSERT() macro to be defined!
#endif

#include <algorithm>

// find_or_add
template<class Container, class T> typename Container::const_iterator
find_or_add(Container& c, const T& t)
{
  typename Container::const_iterator i = find(c.begin(), c.end(), t);
  if (i == c.end())
    {
      c.push_back(t);

      // TODO: this is inefficient; fixme, but how?
      i = find(c.begin(), c.end(), t);
      ASSERT(i != c.end());	// push_back() always succeeds.
    }
  return i;
}

#endif

/* Local variables: */
/* mode: c++ */
/* End: */
