/*
 * seqstate.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998, Free Software Foundation, Inc. 
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
 * $Id: seqstate.h,v 1.11 1999/04/18 17:39:41 james Exp $
 *
 */

#ifndef CSSC__SEQSTATE_H__
#define CSSC__SEQSTATE_H__

#include "stack.h"
#include <list>


class cssc_delta_table;

/* #define USE_OLD_SEQSTATE 1 */

/* This class is used to decide which lines of the body of a SCCS file
   should be included in a gotten file. */

#ifdef USE_OLD_SEQSTATE


class seq_state
{
  enum flags
  {
    INCLUDED    = 001,
    EXCLUDED    = 002,
    MASK        = 003,
    PREDECESSOR = 004,
    EXPLICIT    = 010,
    ACTIVE      = 020,
    INSERTING   = 040
  };

  unsigned char *p;
  seq_no last;
  int deleting;
  stack<seq_no> active_stack;
  seq_no active;

  void
  copy(class seq_state const &it) {
    last = it.last;
    p = new unsigned char[last + 1];
    memcpy(p, it.p, sizeof(unsigned char) * (last + 1));
    deleting = it.deleting;
    active = it.active;
  }

public:
  seq_state(seq_no l): active_stack(l + 1) {
    p = new unsigned char[l + 1];
    for( int i=0; i<l+1; i++)
      p[i] = '\0';
    last = l;
    deleting = 0;
    active = 0;
  }

  seq_state(class seq_state const &it): active_stack(it.active_stack) {
    copy(it);
  }

  class seq_state &
  operator =(class seq_state const &it) {
    if (this != &it) {
      delete [] p;
      copy(it);
      active_stack = it.active_stack;
    }
    return *this;
  }

  int
  is_included(seq_no seq) const { 
    return (p[seq] & MASK) == INCLUDED;
  }

  int
  is_excluded(seq_no seq) const { 
    return (p[seq] & MASK) == EXCLUDED;
  }

  int
  is_predecessor(seq_no seq) const {
    return (p[seq] & PREDECESSOR) != 0;
  }

  int
  is_explicit(seq_no seq) const {
    return (p[seq] & EXPLICIT) != 0;
  }
        
  void
  set_predecessor(seq_no seq) {
    ASSERT(seq > 0 && seq <= last);
    if ((p[seq] & MASK) == 0) {
      p[seq] |= INCLUDED;
    }
    p[seq] |= PREDECESSOR;
  }

  void
  pred_include(seq_no seq) {
    ASSERT(seq > 0 && seq <= last);
    p[seq] = (p[seq] & ~MASK) | INCLUDED;
  }

  void
  pred_exclude(seq_no seq) {
    ASSERT(seq > 0 && seq <= last);
    p[seq] = (p[seq] & ~MASK) | EXCLUDED;
  }

  void
  include(seq_no seq) {
    ASSERT(seq > 0 && seq <= last);
    if (!is_included(seq)) {
      p[seq] = (p[seq] & ~MASK) | INCLUDED | EXPLICIT;
    }
  }

  void
  exclude(seq_no seq) {
    ASSERT(seq > 0 && seq <= last);
    if (is_included(seq)) {
      p[seq] = (p[seq] & ~MASK) | EXCLUDED | EXPLICIT;
    }

  }

  const char * start(seq_no seq, int insert);
  const char * end(seq_no seq);
  seq_no active_seq() const;

  void set_deleting_flag();
  void mark_our_deltas(seq_no me, const cssc_delta_table &dt);
  
  int include_line() const { return deleting == 0; }

  ~seq_state() {
    delete [] p;
  }
};

#else


////////////////////////////////////////// New seqstate implementation.

#endif


class seq_state
{
  struct action
  {
    seq_no seq;
    char   command;

    action(seq_no s, char c) : seq(s), command(c) { }
  };
  
  // Make assignment and copy constructor private.
  const seq_state& operator=(const seq_state& s);
  
  unsigned char * pIncluded;
  unsigned char * pExcluded;
  unsigned char * pExplicit;
  
  seq_no          last;
  seq_no          active; // for use by "get -m" and so on.


  // We keep a record of the open ^AI or ^AD expressions
  // that are currently in effect, while reading the SCCS file.
  list<action>    active_actions;
  action          current_action;
  
  bool            inserting;	// current state.


  // Calculate a new value for the "inserting" flag.
  void decide_disposition();

  action default_action() const;
  
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

