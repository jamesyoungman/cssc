/*
 * delta.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999 Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 *
 * Definition of the class sccs_delta.
 *
 * $Id: delta.h,v 1.5 2007/06/19 23:14:45 james_youngman Exp $
 *
 */


#ifndef CSSC_DELTA_H
#define CSSC_DELTA_H "$Id: delta.h,v 1.5 2007/06/19 23:14:45 james_youngman Exp $"

struct delta
{
  unsigned long inserted, deleted, unchanged;
  char type;
  sid id;
  sccs_date date;
  mystring user;
  seq_no seq, prev_seq;
  mylist<seq_no> included, excluded, ignored;
  
  // have_* are a hack to ensure that prt works the same way
  // as the Real Thing.  We have to output Excludes: lines
  // if the SCCS file contained even an EMPTY includes list.
  bool have_includes, have_excludes, have_ignores;
  mylist<mystring> mrs;
  mylist<mystring> comments;
  
  delta()
    : have_includes(false),
      have_excludes(false),
      have_ignores (false)
  {
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	mylist<mystring> ms, mylist<mystring> cs)
    : type(t), id(i), date(d), user(u),
      seq(s), prev_seq(p),
      have_includes(false), have_excludes(false),
      have_ignores(false),
      mrs(ms), comments(cs)
  {
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	mylist<seq_no> incl, mylist<seq_no> excl,
	mylist<mystring> ms, mylist<mystring> cs)
    : type(t), id(i), date(d), user(u),
      seq(s), prev_seq(p),
      included(incl), excluded(excl),
      have_includes(false), have_excludes(false),
      have_ignores(false),
      mrs(ms), comments(cs)
  {
  }
  
  
  delta &operator =(delta const &);
  bool removed() const;
  private:
  delta(struct delta const &); /* undefined */
};


#endif /* CSSC_DELTA_H */

/* Local variables: */
/* mode: c++ */
/* End: */
