/*
 * seqstate.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999 Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Defines the class seqstate.  
 *
 * $Id: seqstate.h,v 1.13 1999/04/21 22:18:24 james Exp $
 *
 */

#ifndef CSSC__SEQSTATE_H__
#define CSSC__SEQSTATE_H__

#include "stack.h"

class cssc_delta_table;


/* This class is used to decide which lines of the body of a SCCS file
 * should be included in a gotten file.
 */


class seq_state
{
  // Make assignment and copy constructor private.
  const seq_state& operator=(const seq_state& s);
  
  unsigned char * pIncluded;
  unsigned char * pExcluded;
  unsigned char * pExplicit;
  
  unsigned char * pActive;
  char          * pCommand;
  
  seq_no          last;
  seq_no          active; // for use by "get -m" and so on.


  // We keep a record of the open ^AI or ^AD expressions
  // that are currently in effect, while reading the SCCS file.
  // This is kept in the pActive array.  If pActive[n] true,
  // then pCommand[i] contains an 'I' or 'D' indicating that a 
  // ^AI or ^AD had been encountered.

  bool            inserting;	// current state.


  // Calculate a new value for the "inserting" flag.
  void decide_disposition();

public:
  
  seq_state(seq_no l);
  seq_state(const seq_state& s);
  ~seq_state();
  
  bool is_included(seq_no) const;
  bool is_excluded(seq_no) const;

  bool is_explicitly_tagged(seq_no) const;

  void set_explicitly_included(seq_no);
  void set_explicitly_excluded(seq_no);
  void set_included(seq_no);
  void set_excluded(seq_no);
  
  // stuff for use when reading the body of the s-file.

  // When we find ^AI or ^AD
  const char * start(seq_no seq, char command);

  // When we find ^AE.
  const char * end(seq_no seq);

  // Tells us if the delta at the top of the stack is being included.
  int include_line() const;


  // finding out which seq is active, currently.
  seq_no active_seq() const;
};



#endif

/* Local variables: */
/* mode: c++ */
/* End: */

