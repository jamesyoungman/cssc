/*
 * delta.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 *
 * Definition of the class sccs_delta.
 *
 * $Id: delta.h,v 1.1 1997/11/30 21:08:09 james Stab $
 *
 */


#ifndef CSSC_DELTA_H
#define CSSC_DELTA_H "$Id: delta.h,v 1.1 1997/11/30 21:08:09 james Stab $"

struct delta
{
  unsigned long inserted, deleted, unchanged;
  char type;
  sid id;
  sccs_date date;
  mystring user;
  seq_no seq, prev_seq;
  list<seq_no> included, excluded, ignored;
  
  // have_* are a hack to ensure that prt works the same way
  // as the Real Thing.  We have to output Excludes: lines
  // if the SCCS file contained even an EMPTY includes list.
  bool have_includes, have_excludes, have_ignores;
  list<mystring> mrs;
  list<mystring> comments;
  
  delta()
    : have_includes(false),
      have_excludes(false),
      have_ignores (false)
  {
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	list<mystring> ms, list<mystring> cs)
    : type(t), id(i), date(d), user(u),
      seq(s), prev_seq(p),
      have_includes(false), have_excludes(false),
      have_ignores(false),
      mrs(ms), comments(cs)
  {
  }
  
  delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
	list<seq_no> incl, list<seq_no> excl,
	list<mystring> ms, list<mystring> cs)
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
