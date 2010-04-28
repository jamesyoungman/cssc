/*
 * delta.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2007 Free Software Foundation, Inc. 
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
 *
 * Definition of the class delta.
 */


#ifndef CSSC_DELTA_H
#define CSSC_DELTA_H 1

#include "sid.h"
#include "sccsdate.h"
#include "mystring.h"
#include "mylist.h"

typedef unsigned short seq_no;

class delta
{
  char delta_type_;
  sid id_;
  sccs_date date_;
  mystring user_;
  seq_no seq_, prev_seq_;
  // have_* are a hack to ensure that prt works the same way
  // as the Real Thing.  We have to output Excludes: lines
  // if the SCCS file contained even an EMPTY includes list.
  bool have_includes_, have_excludes_, have_ignores_;
  mylist<seq_no> included_, excluded_, ignored_;
  mylist<mystring> mrs_;
  mylist<mystring> comments_;
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
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	mylist<mystring> ms, mylist<mystring> cs)
    : delta_type_(t), id_(i), date_(d), user_(u),
      seq_(s), prev_seq_(p),
      have_includes_(false), have_excludes_(false),
      have_ignores_(false),
      mrs_(ms), comments_(cs),
      inserted_(0u),
      deleted_(0u),
      unchanged_(0u)
  {
    ASSERT(is_valid_delta_type(delta_type_));
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	mylist<seq_no> incl, mylist<seq_no> excl,
	mylist<mystring> ms, mylist<mystring> cs)
    : delta_type_(t), id_(i), date_(d), user_(u),
      seq_(s), prev_seq_(p),
      included_(incl), excluded_(excl),
      have_includes_(incl.length() > 0), have_excludes_(excl.length() > 0),
      have_ignores_(false),
      mrs_(ms), comments_(cs),
      inserted_(0u),
      deleted_(0u),
      unchanged_(0u)
  {
    ASSERT(is_valid_delta_type(delta_type_));
  }

  inline const sid& id() const { return id_; }
  void set_id(const sid& id) { id_ = id; }
    
  inline const sccs_date& date() const { return date_; }
  void set_date(const sccs_date& d) { date_ = d; }

  inline const mystring& user() const {return user_; }
  void set_user(const mystring& u) { user_ = u; }

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
  
  const mylist<seq_no>& get_included_seqnos() const
  {
    return included_;
  }
  
  const mylist<seq_no>& get_excluded_seqnos() const
  {
    return excluded_;
  }
  
  const mylist<seq_no>& get_ignored_seqnos() const
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
	ASSERT(included_.length() == 0);
      }
    have_includes_ = val;
  }

  void add_include(const seq_no &s)
  {
    included_.add(s);
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
	ASSERT(excluded_.length() == 0);
      }
    have_excludes_ = val;
  }

  void add_exclude(const seq_no &s)
  {
    excluded_.add(s);
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
	ASSERT(ignored_.length() == 0);
      }
    have_ignores_ = val;
  }

  void add_ignore(const seq_no &s)
  {
    ignored_.add(s);
    have_ignores_ = true;
  }

  const mylist<mystring>& mrs() const 
  { 
    return mrs_; 
  }

  void set_mrs(const mylist<mystring>& mrs)
  {
    mrs_ = mrs;
  }
  
  void add_mr(const mystring& s)
  {
    mrs_.add(s);
  }
  
  const mylist<mystring>& comments() const
  {
    return comments_;
  }

  void set_comments(const mylist<mystring>& comments) 
  {
    comments_ = comments;
  }

  void add_comment(const mystring& s)
  {
    comments_.add(s);
  }
  
  delta &operator =(delta const &);
  
  bool removed() const;
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
