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
 * Definition of the class sccs_delta.
 */


#ifndef CSSC_DELTA_H
#define CSSC_DELTA_H 1

struct delta
{
  unsigned long inserted, deleted, unchanged;
  sid id;
  sccs_date date;
  mystring user;
  seq_no seq, prev_seq;
  mylist<seq_no> included, excluded, ignored;
private:
  char delta_type;

public:
  // have_* are a hack to ensure that prt works the same way
  // as the Real Thing.  We have to output Excludes: lines
  // if the SCCS file contained even an EMPTY includes list.
  bool have_includes, have_excludes, have_ignores;
  mylist<mystring> mrs;
  mylist<mystring> comments;
  
  delta()
    : delta_type('D'),
      id(0),
      seq(0),
      prev_seq(0),
      have_includes(false),
      have_excludes(false),
      have_ignores (false)
  {
    ASSERT(is_valid_delta_type(delta_type));
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	mylist<mystring> ms, mylist<mystring> cs)
    : delta_type(t), id(i), date(d), user(u),
      seq(s), prev_seq(p),
      have_includes(false), have_excludes(false),
      have_ignores(false),
      mrs(ms), comments(cs)
  {
    ASSERT(is_valid_delta_type(delta_type));
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	mylist<seq_no> incl, mylist<seq_no> excl,
	mylist<mystring> ms, mylist<mystring> cs)
    : delta_type(t), id(i), date(d), user(u),
      seq(s), prev_seq(p),
      included(incl), excluded(excl),
      have_includes(incl.length() > 0), have_excludes(excl.length() > 0),
      have_ignores(false),
      mrs(ms), comments(cs)
  {
    ASSERT(is_valid_delta_type(delta_type));
  }
  
  
  delta &operator =(delta const &);
  bool removed() const;
  char get_type() const
  {
    return delta_type;
  }
  void set_type(char t)
  {
    ASSERT(is_valid_delta_type(t));
    delta_type = t;
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

private:
  delta(struct delta const &); /* undefined */
};


#endif /* CSSC_DELTA_H */

/* Local variables: */
/* mode: c++ */
/* End: */
