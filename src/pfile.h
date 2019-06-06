/*
 * pfile.h: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2007, 2008, 2009, 2010, 2011,
 *  2014, 2019 Free Software Foundation, Inc.
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
 * Definition of the class sccs_pfile.
 */

#ifndef CSSC__PFILE_H__
#define CSSC__PFILE_H__

#include <list>
#include <string>
#include <utility>
#include <iterator>

#include "failure.h"
#include "failure_or.h"
#include "mode.h"
#include "sccsname.h"
#include "sid.h"
#include "sccsdate.h"
#include "ioerr.h"

template<typename It, typename Pred> struct filter_iterator
{
  It rep_;
  Pred pred_;

  typedef typename std::iterator_traits<It>::reference ref;
  typedef typename std::iterator_traits<It>::pointer ptr;
  filter_iterator(It val, Pred pred) : rep_(val), pred_(pred) { }
  filter_iterator& operator++()	// preincrement.
  {
    do
      {
	++rep_;
      } while (!pred_(*rep_));
    return *this;
  }

  ref operator*()
  {
    return *rep_;
  }

  ptr operator->()
  {
    return &*rep_;
  }

  bool operator==(const filter_iterator& other)
  {
    return rep_ == other.rep_;
  }

  bool operator!=(const filter_iterator& other)
  {
    return !(rep_ == other.rep_);
  }

};


class sccs_pfile {
public:
  enum class pfile_mode { PFILE_READ, PFILE_APPEND, PFILE_UPDATE };
  enum class find_status { FOUND, NOT_FOUND, AMBIGUOUS };

private:
  struct edit_lock
  {
    sid got, delta;
    std::string user;
    sccs_date date;
    sid_list include, exclude;

    edit_lock(const char *g, const char *d, const char *u,
	      const char *dd, const char *dt, const char *i,
	      const char *x)
      : got(g),
	delta(d),
	user(u),
	date(dd, dt),
	include(i),
	exclude(x)
    {
    }

    edit_lock()
      : got(),
	delta(),
	user(),
	date(),
	include(),
	exclude()
    {
    }
  };

  sccs_name &name_;
  std::string pname_;
  pfile_mode pmode_;

  std::list<edit_lock> edit_locks;
  typedef std::list<edit_lock>::size_type lock_count_type;

  NORETURN corrupt(int lineno, const char *msg) const  POSTDECL_NORETURN;

  cssc::FailureOr<lock_count_type>
  write_edit_locks(FILE *out, const std::string& file_name) const;

public:
  sccs_pfile(sccs_name &name, pfile_mode pmode_);

  typedef std::list<edit_lock>::size_type size_type;
  typedef std::list<edit_lock>::iterator iterator;
  typedef std::list<edit_lock>::const_iterator const_iterator;

  iterator begin() { return edit_locks.begin(); }
  iterator end() { return edit_locks.end(); }

  const_iterator begin() const { return edit_locks.begin(); }
  const_iterator end() const { return edit_locks.end(); }

  size_type length() const { return edit_locks.size(); }

  const_iterator find_locked(const sid& id) const;
  bool is_locked(const sid& id) const
  {
    const_iterator it = find_locked(id);
    return it != end();
  }

  const_iterator find_to_be_created(const sid& id) const;
  bool is_to_be_created(const sid& id) const
  {
    const_iterator it = find_to_be_created(id);
    return it != end();
  }

  cssc::Failure add_lock(sid got, sid delta,
			 sid_list &included, sid_list &excluded);

  std::pair<find_status, iterator> find_sid(const sid& id);
  cssc::Failure  print_lock_sid(FILE *fp, const_iterator pos) const;
  void delete_lock(iterator i) { edit_locks.erase(i); }

  cssc::Failure update(bool pfile_already_exists) const;

  ~sccs_pfile();
};

#endif /* __PFILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
