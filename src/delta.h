/*
 * delta.h: Part of GNU CSSC.
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
 * Definition of the class delta.
 */


#ifndef CSSC_DELTA_H
#define CSSC_DELTA_H 1

#include <set>
#include <string>
#include <vector>
#include "sid.h"
#include "sccsdate.h"

typedef unsigned short seq_no;

class delta
{
  char delta_type_;
  sid id_;
  sccs_date date_;
  std::string user_;
  seq_no seq_, prev_seq_;
  // have_* are a hack to ensure that prt works the same way
  // as the Real Thing.  We have to output Excludes: lines
  // if the SCCS file contained even an EMPTY includes list.
  bool have_includes_, have_excludes_, have_ignores_;
  std::vector<seq_no> included_, excluded_, ignored_;
  std::vector<std::string> mrs_;
  std::vector<std::string> comments_;
  unsigned long inserted_, deleted_, unchanged_;

public:

  delta()
    : delta_type_('D'),
      id_(0),
      seq_(0),
      prev_seq_(0),
      have_includes_(false),
      have_excludes_(false),
      have_ignores_ (false),
      inserted_(0u),
      deleted_(0u),
      unchanged_(0u)
  {
    ASSERT(is_valid_delta_type(delta_type_));
  }

  delta(char t, sid i, sccs_date d, const std::string& u, seq_no s, seq_no p,
	const std::vector<std::string>& mrs, const std::vector<std::string>& cs)
    : delta_type_(t), id_(i), date_(d), user_(u),
      seq_(s), prev_seq_(p),
      have_includes_(false), have_excludes_(false),
      have_ignores_(false),
      mrs_(mrs), comments_(cs),
      inserted_(0u),
      deleted_(0u),
      unchanged_(0u)
  {
    ASSERT(is_valid_delta_type(delta_type_));
  }

  delta(char t, sid i, sccs_date d, const std::string& u, seq_no s, seq_no p,
	const std::set<seq_no>& incl, const std::set<seq_no>& excl,
	const std::vector<std::string>& mrs, const std::vector<std::string>& cs)
    : delta_type_(t), id_(i), date_(d), user_(u),
      seq_(s), prev_seq_(p),
      have_includes_(!incl.empty()), have_excludes_(!excl.empty()),
      have_ignores_(false),
      included_(incl.cbegin(), incl.cend()),
      excluded_(excl.cbegin(), excl.cend()),
      mrs_(mrs), comments_(cs),
      inserted_(0u),
      deleted_(0u),
      unchanged_(0u)
  {
    ASSERT(is_valid_delta_type(delta_type_));
  }

  inline const sid& id() const { return id_; }
  void set_id(const sid& newid) { id_ = newid; }

  inline const sccs_date& date() const { return date_; }
  void set_date(const sccs_date& d) { date_ = d; }

  inline const std::string& user() const {return user_; }
  void set_user(const std::string& u) { user_ = u; }

  inline seq_no seq() const { return seq_; }
  void set_seq(const seq_no& s) { seq_ = s; }

  inline const seq_no& prev_seq() const { return prev_seq_; }
  void set_prev_seq(const seq_no& p) { prev_seq_ = p; }

  unsigned long inserted() const { return inserted_; }
  void set_inserted(unsigned long val) { inserted_ = val; }

  unsigned long deleted() const { return deleted_; }
  unsigned long unchanged()  const { return unchanged_; }

  void set_idu(unsigned long i, unsigned long d, unsigned long u)
  {
    inserted_ = i;
    deleted_ = d;
    unchanged_ = u;
  }

  void increment_inserted()
  {
    ++inserted_;
  }

  void increment_deleted()
  {
    ++deleted_;
  }

  void increment_unchanged()
  {
    ++unchanged_;
  }

  const std::vector<seq_no>& get_included_seqnos() const
  {
    return included_;
  }

  const std::vector<seq_no>& get_excluded_seqnos() const
  {
    return excluded_;
  }

  const std::vector<seq_no>& get_ignored_seqnos() const
  {
    return ignored_;
  }

  bool has_includes() const
  {
    return have_includes_;
  }

  void set_has_includes(bool val)
  {
    if (!val)
      {
	ASSERT(included_.empty());
      }
    have_includes_ = val;
  }

  void add_include(const seq_no &s)
  {
    included_.push_back(s);
    have_includes_ = true;
  }

  bool has_excludes() const
  {
    return have_excludes_;
  }

  void set_has_excludes (bool val)
  {
    if (!val)
      {
	ASSERT(excluded_.empty());
      }
    have_excludes_ = val;
  }

  void add_exclude(const seq_no &s)
  {
    excluded_.push_back(s);
    have_excludes_ = true;
  }

  bool has_ignores() const
  {
    return have_ignores_;
  }

  void set_has_ignores (bool val)
  {
    if (!val)
      {
	ASSERT(ignored_.empty());
      }
    have_ignores_ = val;
  }

  void add_ignore(const seq_no &s)
  {
    ignored_.push_back(s);
    have_ignores_ = true;
  }

  const std::vector<std::string>& mrs() const
  {
    return mrs_;
  }

  void set_mrs(const std::vector<std::string>& updated_mrs)
  {
    mrs_ = updated_mrs;
  }

  void add_mr(const std::string& s)
  {
    mrs_.push_back(s);
  }

  const std::vector<std::string>& comments() const
  {
    return comments_;
  }

  void set_comments(const std::vector<std::string>& updated_comments)
  {
    comments_ = updated_comments;
  }

  void prepend_comments(const std::vector<std::string>& prefix)
  {
    comments_.insert(comments_.begin(), prefix.begin(), prefix.end());
  }

  void add_comment(const std::string& s)
  {
    comments_.push_back(s);
  }

  delta &operator =(delta const &);

  bool removed() const
  {
    return 'R' == delta_type_;
  }

  char get_type() const
  {
    return delta_type_;
  }

  void set_type(char t)
  {
    ASSERT(is_valid_delta_type(t));
    delta_type_ = t;
  }


  static bool is_valid_delta_type(char t)
  {
    switch (t)
      {
      case 'D':
      case 'R':
	return true;
      default:
	return false;
      }
  }
};


#endif /* CSSC_DELTA_H */

/* Local variables: */
/* mode: c++ */
/* End: */
