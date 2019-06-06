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
  seq_no high_seqno_;
  sid high_release_;
  std::vector<struct delta> items_;
  std::map<seq_no, size_t> seq_table_;

protected:
  void update_highest(const delta& d)
  {
    if (d.seq() > high_seqno_)
      high_seqno_ = d.seq();

    if (!d.removed())
      {
	if (high_release_.is_null())
	  {
	    if (d.id().on_trunk())
	      high_release_ = d.id();
	  }
	else if (d.id() > high_release_)
	  {
	    high_release_ = d.id();
	  }
      }
  }


public:
  typedef std::vector<struct delta>::size_type size_type;

  stl_delta_list()
    : high_seqno_(0),
      high_release_(sid::null_sid()),
      items_(),
      seq_table_()
  {
  }

  seq_no get_high_seqno() const
  {
    return high_seqno_;
  }

  const sid& get_high_release() const
  {
    return high_release_;
  }

  size_type size() const
  {
    return items_.size();
  }

  const delta& at(size_type i) const
  {
    return items_.at(i);
  }

  delta& at(size_type i)
  {
    return items_.at(i);
  }

  void add(const delta& d)
  {
    size_t pos = items_.size();
    items_.push_back(d);
    seq_table_[d.seq()] = pos;
    update_highest(d);
  }

  stl_delta_list& operator += (const stl_delta_list& other)
  {
    for (size_type i=0; i<other.size(); ++i)
      {
	add(other.at(i));
      }
    return *this;
  }

  bool delta_at_seq_exists(seq_no seq) const
  {
    std::map<seq_no, size_t>::const_iterator i = seq_table_.find(seq);
    return i != seq_table_.end();
  }

  const delta& delta_at_seq(seq_no seq) const
  {
    std::map<seq_no, size_t>::const_iterator i = seq_table_.find(seq);
    ASSERT (i != seq_table_.end());
    return items_[i->second];
  }
};


class cssc_delta_table
{
  typedef stl_delta_list delta_list;
  delta_list l_;

  cssc_delta_table &operator =(cssc_delta_table const &); /* undefined */
  cssc_delta_table(cssc_delta_table const &); /* undefined */

public:
  typedef stl_delta_list::size_type size_type;

  cssc_delta_table()
    : l_()
  {
  }

  void add(const delta &d);
  void prepend(const delta &); /* sf-add.c */

  bool delta_at_seq_exists(seq_no seq) const;
  const delta & delta_at_seq(seq_no seq) const;

  const delta *find(sid id) const;
  const delta *find_any(sid id) const; // includes removed deltas.
  delta *find(sid id);

  seq_no highest_seqno() const { return l_.get_high_seqno(); }
  seq_no next_seqno()    const;
  sid highest_release() const { return l_.get_high_release(); }

  size_type size() const { return l_.size(); }

  const delta& at(size_type pos) const { return l_.at(pos); }
  delta& at(size_type pos) { return l_.at(pos); }

  ~cssc_delta_table();
};


#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
