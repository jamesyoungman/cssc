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

#if 0
template<class TYPE>
using mylist = std::vector<TYPE>;
#else
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

  template <class InputIterator>
  mylist(InputIterator first, InputIterator last)
    : items_(first, last)
  {
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

  void push_back(TYPE const &ent) { items_.push_back(ent); }

  size_type size() const { return items_.size(); }
  bool empty() const { return items_.empty(); }

  TYPE const &
  operator [](size_type index) const
    {
      ASSERT(index < items_.size());
      return items_[index];
    }

  template <class InputIterator>
  iterator insert(const_iterator position, InputIterator first, InputIterator last) 
  {
    return items_.insert(position, first, last);
  }

  void clear() { items_.clear(); }
  iterator begin() { return items_.begin(); }
  const_iterator begin() const { return items_.begin(); }
  const_iterator cbegin() const { return items_.cbegin(); }
  iterator end() { return items_.end(); }
  const_iterator end() const { return items_.end(); }
  const_iterator cend() const { return items_.cend(); }

  ~mylist()
  {
  }
};
#endif

#endif /* __LIST_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
