/*
 * delta-iterator.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019, 2024
 *  Free Software Foundation, Inc.
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
 *
 * Definition of the class delta_iterator.
 */


#ifndef CSSC_DELTA_ITERATOR_H
#define CSSC_DELTA_ITERATOR_H 1

#include <memory>

#include "delta-table.h"

class delta;

enum class delta_selector
  {
    current,			// non-deleted deltas
    all				// all deltas
  };

class delta_iterator
{
  cssc_delta_table *dtbl_;
  cssc_delta_table::size_type pos_;
  bool first_;

public:
  delta_iterator(cssc_delta_table*, delta_selector);

  bool next();			// returns false when exhausted.
  cssc_delta_table::size_type index() const;

  delta * operator->();
  delta& operator*();
  const delta * operator ->() const;
  const delta& operator*() const;

private:
  inline cssc_delta_table::size_type advance()
  {
    if (first_)
      first_ = false;
    else
      ++pos_;
    return pos_;
  }

  inline bool all()  const
  {
    return selector_ == delta_selector::all;
  }

  delta_selector selector_;
};

class const_delta_iterator
{
  const cssc_delta_table *dtbl_;
  cssc_delta_table::size_type pos_;
  bool first_;

public:
  const_delta_iterator(const cssc_delta_table*, delta_selector);

  bool next();
  cssc_delta_table::size_type index() const;

  delta const * operator ->() const;
  const delta& operator*() const;

private:
  inline cssc_delta_table::size_type advance()
  {
    if (first_)
      first_ = false;
    else
      ++pos_;
    return pos_;
  }

  inline bool all()  const
  {
    return selector_ == delta_selector::all;
  }

  delta_selector selector_;
};

#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
