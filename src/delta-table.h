/*
 * delta-table.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014, 2019
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
 * Definition of the classes cssc_delta_table and delta_iterator.
 */


#ifndef CSSC_DELTA_TABLE_H
#define CSSC_DELTA_TABLE_H 1

#include <deque>
#include <vector>
#include <map>

#include "delta.h"

class stl_delta_list
{
  seq_no high_seqno;
  sid high_release;
  std::vector<struct delta> items;
  std::map<seq_no, size_t> seq_table;

protected:
  void update_highest(const delta& d)
  {
    if (d.seq() > high_seqno)
      high_seqno = d.seq();

    if (!d.removed())
      {
	if (high_release.is_null())
	  {
	    if (d.id().on_trunk())
	      high_release = d.id();
	  }
	else if (d.id() > high_release)
	  {
	    high_release = d.id();
	  }
      }
  }


public:
  typedef std::vector<struct delta>::size_type size_type;

  stl_delta_list()
    : high_seqno(0),
      high_release(sid::null_sid())
  {
  }

  seq_no get_high_seqno() const
  {
    return high_seqno;
  }

  const sid& get_high_release() const
  {
    return high_release;
  }

  size_type length() const
  {
    return items.size();
  }

  const delta& select(size_type i) const
  {
    return items[i];
  }

  delta& select(size_type i)
  {
    return items[i];
  }

  void add(const delta& d)
  {
    size_t pos = items.size();
    items.push_back(d);
    seq_table[d.seq()] = pos;
    update_highest(d);
  }

  stl_delta_list& operator += (const stl_delta_list& other)
  {
    for (size_type i=0; i<other.length(); ++i)
      {
	add(other.select(i));
      }
    return *this;
  }

  bool delta_at_seq_exists(seq_no seq) const
  {
    std::map<seq_no, size_t>::const_iterator i = seq_table.find(seq);
    return i != seq_table.end();
  }

  const delta& delta_at_seq(seq_no seq) const
  {
    std::map<seq_no, size_t>::const_iterator i = seq_table.find(seq);
    ASSERT (i != seq_table.end());
    return items[i->second];
  }
};


class cssc_delta_table
{
  typedef stl_delta_list delta_list;
  delta_list l;

  cssc_delta_table &operator =(cssc_delta_table const &); /* undefined */
  cssc_delta_table(cssc_delta_table const &); /* undefined */

public:
  typedef stl_delta_list::size_type size_type;

  cssc_delta_table()
  {
  }

  void add(const delta &d);
  void prepend(const delta &); /* sf-add.c */

  // These two methods should b const, but are not because they
  // call build_seq_table().
  bool delta_at_seq_exists(seq_no seq) const;
  const delta & delta_at_seq(seq_no seq) const;

  const delta *find(sid id) const;
  const delta *find_any(sid id) const; // includes removed deltas.
  delta *find(sid id);

  seq_no highest_seqno() const { return l.get_high_seqno(); }
  seq_no next_seqno()    const;
  sid highest_release() const { return l.get_high_release(); }

  size_type length() const { return l.length(); }

  const delta& select(size_type pos) const { return l.select(pos); }
  delta& select(size_type pos) { return l.select(pos); }

  ~cssc_delta_table();
};


#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
