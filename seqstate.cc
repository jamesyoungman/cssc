/*
 * seqstate.cc: Part of GNU CSSC.
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
 * Defines the non-inline members of the class seq_state.
 *
 */

#include "cssc.h"
#include "seqstate.h"


#define NEW_DELETE_STATE_CODE



seq_no seq_state::active_seq() const
{
  return active;
}



#ifdef NEW_DELETE_STATE_CODE


static int debug = 0;

// New delete state code.

enum arghhh
{
  INCLUDED    = 001,
  EXCLUDED    = 002,
  MASK        = 003,
  PREDECESSOR = 004,
  EXPLICIT    = 010,
  ACTIVE      = 020,
  INSERTING   = 040
};

    static void show_disp(int flags)
{
  if (flags & INCLUDED)
    fprintf(stderr, "INCLUDED ");
  if (flags & EXPLICIT)
    fprintf(stderr, "EXPLICIT ");
  if (flags & PREDECESSOR)
    fprintf(stderr, "PREDECESSOR ");
  if (flags & EXPLICIT)
    fprintf(stderr, "EXPLICIT ");
  if (flags & ACTIVE)
    fprintf(stderr, "ACTIVE ");
  if (flags & INSERTING)
    fprintf(stderr, "INSERTING");
}


void
seq_state::set_deleting_flag()
{
  seq_no insert_in_this_delta       = 0;
  seq_no insert_in_some_other_delta = 0;
  seq_no delete_in_this_delta       = 0;

  for (int i=0; i<=last; ++i)
    {
      const int disposition = p[i];

      if (debug)
	{
	  fprintf(stderr, "seq %d: disposition %o ", i, (unsigned)disposition);
	  show_disp(disposition);
	  fprintf(stderr, "\n");
	}

      if (disposition & ACTIVE)
	{
	  if (disposition & INSERTING)
	    {
	      if (disposition & INCLUDED)
		insert_in_this_delta = i;
	      else
		insert_in_some_other_delta = i;
	    }
	  else if (disposition & INCLUDED)
	    {
	      delete_in_this_delta = i;
	    }
	}
    }

  if (delete_in_this_delta >= insert_in_this_delta)
    {
      deleting = 1;		// we don't want it.
    }
  else if (insert_in_this_delta > insert_in_some_other_delta)
    {
      deleting = 0;		// we want it.
      active = insert_in_this_delta;
    }
  else
    {
      deleting = 1;		// we can't have it.
    }
  
  if (debug)
    {
      fprintf(stderr, "delete_in_this_delta = %d\n",
	      delete_in_this_delta);
      fprintf(stderr, "insert_in_some_other_delta = %d\n",
	      insert_in_some_other_delta);
      fprintf(stderr, "insert_in_this_delta = %d\n",
	      insert_in_some_other_delta);
      fprintf(stderr, "deleting=%d, active=%u",
	      (int)deleting, (unsigned)active);
    }
  
}



const char *
seq_state::start(seq_no seq, int insert)
{
  ASSERT(seq > 0 && seq <= last);

  if (debug)
    {
      fprintf(stderr, "start(seq=%u, insert=%d)\n", (unsigned)seq, insert);
    }
  
  if (insert)
    {
      active_stack.push(active);
      active = seq;
    }
  
  
  if (p[seq] & ACTIVE)
    return "Seq already active.";
  
  p[seq] |= ACTIVE;
  if (insert)
    p[seq] |= INSERTING;
  else
    p[seq] &= ~INSERTING;

  set_deleting_flag();
  return NULL;
}

const char *
seq_state::end(seq_no seq)
{
  ASSERT(seq > 0 && seq <= last);
  
  if (!(p[seq] & ACTIVE))
    return "Seq not active.";
  
  p[seq] &= ~(ACTIVE | INSERTING);
  
  if (seq == active)
    {
      active = active_stack.pop();
    }

  set_deleting_flag();
  return NULL;
}


#else
// Old delete state code.

const char *
seq_state::start(seq_no seq, int insert)
{
  ASSERT(seq > 0 && seq <= last);

  if (insert)
    {
      active_stack.push(active);
      active = seq;
    }
  
  if (!is_included(seq))
    {
      insert = !insert;
    }
  
  if (p[seq] & ACTIVE)
    {
      return "Seq already active.";
    }
  
  p[seq] |= ACTIVE;
  
  if (insert)
    {
      p[seq] |= INSERTING;
    }
  else
    {
      p[seq] &= ~INSERTING;
      deleting++;
    }
  
  return NULL;
}

const char *
seq_state::end(seq_no seq)
{
  ASSERT(seq > 0 && seq <= last);
  
  if (!(p[seq] & ACTIVE))
    {
      return "Seq not active.";
    }
  
  p[seq] &= ~ACTIVE;
  
  int insert = ((p[seq] & INSERTING) != 0);
  
  if (!insert)
    {
      ASSERT(deleting > 0);
      deleting--;
    }
  
  if (!is_included(seq) ^ insert)
    {
      if (seq == active)
	{
	  active = active_stack.pop();
	}
      else
	{
	  return "Overlapping insertions";
	}
    }
  
  return NULL;
}
#endif
