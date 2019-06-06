/*
 * sid.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 *
 * Defines the classes sid, and release as well as the typedef's sid_list
 * and release_list.
 */

#ifndef CSSC__SID_H__
#define CSSC__SID_H__

#include <ostream>
#include <string>

#include "cssc-assert.h"
#include "failure.h"
#include "sid_list.h"

#include "release.h"

#include "relvbr.h"


class sccs_file;

class sid
{
public:
  sid(short r, short l, short b, short s)
    : rel_(r), level_(l), branch_(b), sequence_(s)
  {
    ASSERT((!r && !l && !b && !s)
	   || (r && !l && !b && !s)
	   || (r && l && !b && !s)
	   || (r && l && b));
  }

  static sid null_sid();
  sid(): rel_(-1), level_(0), branch_(0), sequence_(0) {}
  sid(const char *s);
  sid(release);		/* Defined below */
  sid(relvbr);		/* Defined below */

  bool is_null() const { return rel_ <= 0; }
  bool gte(sid const &id) const; // used by sccs_file::find_requested_sid().

  release get_release() const;
  relvbr get_relvbr() const;

  sid(sid const &id)
    : rel_(id.rel_), level_(id.level_),
      branch_(id.branch_), sequence_(id.sequence_)
  {
  }

  sid &
  operator=(sid const &id)
  {
    rel_ = id.rel_;
    level_ = id.level_;
    branch_ = id.branch_;
    sequence_ = id.sequence_;
    return *this;
  }

  bool valid() const { return rel_ > 0; }

  bool
  partial_sid() const
  {
    return level_ == 0 || (branch_ != 0 && sequence_ == 0);
  }

  int components() const;
  bool on_trunk() const;

  bool operator>(sid const &i2) const
  {
    return comparable(i2) && gt(i2);
  }

  bool operator>=(sid const &i2) const
  {
    return comparable(i2) && gte(i2);
  }

  bool operator<(sid const &i2) const
  {
    return comparable(i2) && !gte(i2);
  }

  bool operator<=(sid const &i2) const
  {
    return comparable(i2) && !gt(i2);
  }

  bool operator==(sid const &i2) const
  {
    return rel_ == i2.rel_
      && level_ == i2.level_
      && branch_ == i2.branch_
      && sequence_ == i2.sequence_;
  }

  bool operator!=(sid const &i2) const
  {
    return rel_ != i2.rel_
      || level_ != i2.level_
      || branch_ != i2.branch_
      || sequence_ != i2.sequence_;
  }

  sid successor() const;

  sid &
  next_branch()
  {
    branch_++;
    sequence_ = 1;
    return *this;
  }

  const sid &
  next_level()
  {
    ++level_;
    branch_ = sequence_ = 0;
    return *this;
  }

  sid &
  operator++()
  {
    if (branch_ != 0)
      {
	sequence_++;
      }
    else if (level_ != 0)
      {
	level_++;
      }
    else
      {
	rel_++;
      }
    return *this;
  }

  sid &
  operator--()
  {
    if (branch_ != 0)
      {
	sequence_--;
      }
    else if (level_ != 0)
      {
	level_--;
      }
    else
      {
	rel_--;
      }
    return *this;
  }

  bool
  is_trunk_successor(sid const &id) const
  {
    return branch_ == 0 && *this < id;
  }

  bool
  branch_greater_than(sid const &id) const
  {
    return rel_ == id.rel_ && level_ == id.level_
      && branch_ > id.branch_;
  }

  bool partial_match(sid const &id) const;
  bool matches(const sid &m, int nfields) const;

  bool
  release_only() const
  {
    return rel_ != 0 && level_ == 0;
  }

  bool
  trunk_match(sid const &id) const
  {
    return rel_ == 0
      || (rel_ == id.rel_ && (level_ == 0
			    || level_ == id.level_));
  }

  cssc::Failure print(FILE *f) const;
  cssc::Failure printf(FILE *f, char fmt, bool force_zero=false) const;

  cssc::Failure
  dprint(FILE *f) const
  {
    if (fprintf(f, "%d.%d.%d.%d", rel_, level_, branch_, sequence_) < 0)
      {
	return cssc::make_failure_from_errno(errno);
      }
    return cssc::Failure::Ok();
  }

  // as_string prints the human-readable version of the sid.
  std::string as_string() const;
  std::ostream& ostream_insert(std::ostream&) const;

private:
  short rel_, level_, branch_, sequence_;

  int comparable(sid const &id) const;
  int gt(sid const &id) const;
};

inline std::ostream& operator<<(std::ostream& os, const sid& s)
{
  return s.ostream_insert(os);
}

inline sid::sid(release r): rel_(r), level_(0), branch_(0), sequence_(0) {}

inline bool operator>(release i1, sid const &i2) { return i1 > i2.get_release(); }
inline bool operator<(release i1, sid const &i2) { return i1 < i2.get_release(); }
inline bool operator>=(release i1, sid const &i2) { return i1 >= i2.get_release(); }
inline bool operator<=(release i1, sid const &i2) { return i1 <= i2.get_release(); }
inline bool operator==(release i1, sid const &i2) { return i1 == i2.get_release(); }
inline bool operator !=(release i1, sid const &i2) { return i1 != i2.get_release(); }

inline bool operator>(sid const &i1, release i2) { return i1.get_release() > i2; }
inline bool operator<(sid const &i1, release i2) { return i1.get_release() < i2; }
inline bool operator>=(sid const &i1, release i2) { return i1.get_release() >= i2; }
inline bool operator<=(sid const &i1, release i2) { return i1.get_release() <= i2; }
inline bool operator ==(sid const &i1, release i2) { return i1.get_release() == i2; }
inline bool operator !=(sid const &i1, release i2) { return i1.get_release() != i2; }

typedef range_list<sid> sid_list;


#endif /* __SID_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
