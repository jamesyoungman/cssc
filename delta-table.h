/*
 * delta-table.h: Part of GNU CSSC.
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 *
 * Definition of the classes cssc_delta_table and delta_iterator.
 *
 * $Id: delta-table.h,v 1.4 1999/04/18 17:59:39 james Exp $
 *
 */


#ifndef CSSC_DELTA_TABLE_H
#define CSSC_DELTA_TABLE_H "$Id: delta-table.h,v 1.4 1999/04/18 17:59:39 james Exp $"

#include "delta.h"

class cssc_delta_table
{
  mylist<struct delta> l;
  int *seq_table;
  seq_no high_seqno;
  sid high_release;
  
  void build_seq_table();
  void update_highest(struct delta const &delta);

  cssc_delta_table &operator =(cssc_delta_table const &); /* undefined */
  cssc_delta_table(cssc_delta_table const &); /* undefined */

public:
  cssc_delta_table()
    : seq_table(NULL),
      high_seqno(0),
      high_release(NULL)
  {
  }

  void add(const delta &d);		
  void prepend(const delta &); /* sf-add.c */

  const delta & delta_at_seq(seq_no seq);
  const delta *find(sid id) const; 
  const delta *find_any(sid id) const; // includes removed deltas.
  delta *find(sid id); 

  seq_no highest_seqno() const { return high_seqno; }
  seq_no next_seqno()    const;
  sid highest_release() const { return high_release; }

  int length() const { return l.length(); }

  const delta& select(int pos) const { return l.select(pos); }
  delta& select(int pos) { return l.select(pos); }
  
  ~cssc_delta_table();
};


#endif /* CSSC_DELTA_TABLE_H */

/* Local variables: */
/* mode: c++ */
/* End: */
