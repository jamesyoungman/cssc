/*
 * rel_list.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 */


#ifndef INC_REL_LIST_H
#define INC_REL_LIST_H

#include <vector>

#include "release.h"


class release_list
{
  std::vector<release> l;

public:
  typedef typename std::vector<release>::size_type size_type;
  typedef typename std::vector<release>::const_iterator const_iterator;

  // Constructors / destructors
  release_list();
  release_list(const release_list& create_from);
  release_list(const char *str);
  ~release_list();

  // Adding and removing members specified in other lists.
  void merge(const release_list& m);
  void remove(const release_list& r);

  // I/O
  cssc::Failure print(FILE *) const;

  // Accessors
  bool empty() const { return l.empty(); }
  bool valid() const { return !empty(); }
  bool member(release r) const;

  const_iterator begin() const { return l.begin(); }
  const_iterator end() const { return l.end(); }
};

/* Local variables: */
/* mode: c++ */
/* End: */
#endif
