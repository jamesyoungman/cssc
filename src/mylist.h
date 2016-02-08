/*
 * mylist.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,1998,2007 Free Software Foundation, Inc.
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
 * Defines the template mylist.
 *
 */

#ifndef CSSC__LIST_H__
#define CSSC__LIST_H__

#include <cstdlib>
#include <algorithm>
#include <vector>
#include <set>

#include "cssc-assert.h"

template <class TYPE>
class mylist
{
protected:
  typedef typename std::vector<TYPE> impl_type;
  impl_type items_;

public:
  typedef typename impl_type::size_type size_type;
  typedef typename impl_type::iterator iterator;
  typedef typename impl_type::const_iterator const_iterator;

  mylist(mylist const &l)
    : items_(l.items_)
  {
  }

  explicit mylist(const std::set<TYPE>& from) 
  {
    std::copy(from.begin(), from.end(), std::back_inserter(items_));
  }

  mylist()
  {
  }

  mylist &operator =(mylist const &l)
  {
    items_ = l.items_;
    return *this;
  }

  bool operator ==(mylist const &l) const
  {
    return items_ == l.items_;
  }

  void
  operator =(void *p)
    {
      ASSERT(p == NULL);
      items_.clear();
    }

  void add(TYPE const &ent)
  {
    items_.push_back(ent);
  }

  void push_back(TYPE const &ent)
  {
    items_.push_back(ent);
  }

  size_type size() const { return items_.size(); }
  size_type length() const { return size(); } // TODO: remove this method.
  bool empty() const { return items_.empty(); }

  TYPE const &
  operator [](size_type index) const
    {
      ASSERT(index < items_.size());
      return items_[index];
    }

  TYPE &
  select(size_type index)
    {
      ASSERT(index >= 0 && index < items_.size());
      return items_[index];
    }

  const mylist<TYPE>& operator+=(const mylist& other)
  {
    typename impl_type::const_iterator ci;
    for (ci=other.items_.begin(); ci!=other.items_.end(); ++ci)
      {
	items_.push_back(*ci);
      }
  }

  // TODO: remove this method
  const mylist<TYPE>& operator-=(const mylist& other)
  {
    if (other.items_.size())
      {
	impl_type remaining;
	std::set<TYPE> unwanted(other.items_.begin(), other.items_.end());

	typename impl_type::const_iterator ci;
	for (ci=items_.begin(); ci!=items_.end(); ++ci)
	  {
	    if (unwanted.find(*ci) == unwanted.end())
	      {
		remaining.push_back(*ci);
	      }
	  }
	items_.swap(remaining);
      }
  }

  iterator begin() { return items_.begin(); }
  const_iterator begin() const { return items_.begin(); }
  iterator end() { return items_.end(); }
  const_iterator end() const { return items_.end(); }

  ~mylist()
  {
  }
};

#endif /* __LIST_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
