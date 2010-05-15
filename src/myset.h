/*
 * myset.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 2002,2004,2007 Free Software Foundation, Inc.
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
 * Defines the template myset.
 *
 */
#ifndef CSSC__MYSETLIST_H__
#define CSSC__MYSETLIST_H__

#include <set>
#include "mylist.h"

template <class TYPE>
class myset
{
  std::set<TYPE> members;

public:
  typedef typename std::set<TYPE>::size_type my_size_type;
  typedef typename std::set<TYPE>::iterator iterator;

    myset()
        {
        }


    my_size_type count() const
        {
            return members.size();
        }

    const mylist<TYPE> list() const
        {
	  mylist<TYPE> result;

	  for (iterator it=members.begin(); it != members.end(); it++)
	    result.add(*it);
	  return result;
        }

    bool is_member(TYPE const &ent) const
        {
	  if (members.find(ent) == members.end())
	    return false;
	  else
	    return true;
        }

    void add(TYPE const &ent)
        {
	  members.insert(ent);
        }
};

#endif
