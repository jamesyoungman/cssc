/*
 * delta-iterator.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014 Free Software Foundation, Inc.
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

class cssc_delta_table;
class delta;

class delta_iterator
{
  cssc_delta_table *dtbl;
  int pos;

public:
  delta_iterator(cssc_delta_table *d);

  int next(int all = 0);
  int index() const;

  delta * operator ->();
  const delta * operator ->() const;

  void rewind();
};

class const_delta_iterator
{
  const cssc_delta_table *dtbl;
  int pos;

public:
  const_delta_iterator(cssc_delta_table const *d);

  int next(int all = 0);
  int index() const;

  delta const * operator ->() const;

  void rewind();
};

#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
