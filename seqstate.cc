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
#include <list>


#undef DEBUG_COMMANDS

#ifdef USE_OLD_SEQSTATE


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


#else

////////////////////////////////////////// new seq_state implementation

seq_state::seq_state(seq_no l) :
  last(l),
  active(0u),
  current_action(default_action())
{
  pIncluded = new unsigned char[l + 1];
  pExcluded = new unsigned char[l + 1];
  pExplicit = new unsigned char[l + 1];
  
  for( int i=0; i<l+1; i++)
    {
      pIncluded[i] = 0;
      pExcluded[i] = 0;
      pExplicit[i] = 0;
    }
  
  decide_disposition();
}


seq_state::seq_state(const seq_state& s) : 
  last(s.last),
  active(s.active),
  active_actions(s.active_actions),
  current_action(s.current_action)
  
{
  pIncluded = new unsigned char[last + 1];
  pExcluded = new unsigned char[last + 1];
  pExplicit = new unsigned char[last + 1];
  
  for( int i=0; i<last+1; i++)
    {
      pIncluded[i] = s.pIncluded[i];
      pExcluded[i] = s.pExcluded[i];
      pExplicit[i] = s.pExplicit[i];
    }

  decide_disposition();
}



bool seq_state::is_included(seq_no n) const
{
  return pIncluded[n];
}

bool seq_state::is_excluded(seq_no n) const
{
  return pExcluded[n];
}

void seq_state::set_explicitly_included(seq_no n)
{
  set_included(n);
  pExplicit[n] = 1;
}

void seq_state::set_explicitly_excluded(seq_no n)
{
  set_excluded(n);
  pExplicit[n] = 1;
}

void seq_state::set_included(seq_no n)
{
  pIncluded[n] = 1;
  pExcluded[n] = 0;
}

void seq_state::set_excluded(seq_no n)
{
  pExcluded[n] = 1;
  pIncluded[n] = 0;
}

bool seq_state::is_explicitly_tagged(seq_no n) const
{
  return pExplicit[n];
}


seq_state::~seq_state()
{
  delete[] pIncluded;
  delete[] pExcluded;
  delete[] pExplicit;
  
  pIncluded = pExcluded = pExplicit = 0;
}

// stuff for use when reading the body of the s-file.


// examine the delta dispositions and the current action,
// and decide if we are currently inserting lines, or not.
//
// We have a series of deltas, each identified by sequence numbers.
// larger sequence numbers were added to the file after smaller
// ones.  Therefore, instructions imposed by larger sequence numbers 
// supercede those imposed by older ones, except when the larger
// sequence number is not included in the delta we are trying to get.
//
// Changes that are inserted by a sequence number later than the one we're
// fetching are considered to be ignored.
void
seq_state::decide_disposition() 
{
  seq_no our_highest_insert         = 0u;
  seq_no our_highest_delete         = 0u;
  seq_no owner_of_current_insertion = 0u;
  
  list<action>::const_iterator i = active_actions.begin();
  while (i != active_actions.end())
    {
      const seq_no s = i->seq;

      if ('I' == i->command)
	{
	  if (is_included(s))
	    {
	      if (our_highest_insert < s)
		our_highest_insert = s;
	    }
	  else
	    {
	      if (owner_of_current_insertion < s)
		owner_of_current_insertion = s;
	    }
	}
      else
	{
	  if (is_included(s))
	    {
	      if (our_highest_delete < s)
		our_highest_delete = s;
	    }
	}

      ++i;
    }

  // If the sequence number of the insert command is later than the
  // sequence number of the delete command, that means that if a
  // delete command was issued for an active delta, it was later
  // countermanded.
  //
  // If the owner of the current insertion is a delta "later" than us,
  // this means that we effectively can't see the insert instruction.
  
  if (our_highest_delete > our_highest_insert)
    {
#if 0
      fprintf(stderr, "Our deletion supercedes insertion\n");
#endif      
      inserting = false;
    }
  else if (our_highest_insert > owner_of_current_insertion)
    {
#if 0
      fprintf(stderr, "Our insertion is in scope and is unsuperceded\n");
#endif      
      inserting = true;
      active = our_highest_insert;
    }
  else
    {
#if 0
      fprintf(stderr, "We have no unsuperceded insertion in scope\n");
#endif      
      inserting = false;
    }
}


// When we find ^AI or ^AD
const char *
seq_state::start(seq_no seq, char command_letter)
{
  // begin diagnostic-only code.
  if (command_letter != 'I' && command_letter != 'D')
    {
      return "invalid command letter";
    }
  
  list<action>::const_iterator i = active_actions.begin();
  while (i != active_actions.end())
    {
      if (i->seq == seq)
	{
	  break;
	}
      ++i;
    }
  
  if  (i != active_actions.end())
    {
      if ('I' == command_letter)
	{
	  return "^AI for sequence number which is already active";
	}
      else
	{
	  return "^AD for sequence number which is already active";
	}
    }
  // end diagnostic-only code.

       
  current_action = action(seq, command_letter);
  active_actions.push_front(current_action);
  decide_disposition();

#ifdef DEBUG_COMMANDS
  fprintf(stderr,
	  "^A%c %u: inserting=%c \n",
	  command_letter,
	  (unsigned) seq,
	  inserting ? 'Y' : 'N');
#endif  
  return NULL;
}

seq_state::action seq_state::default_action() const
{
  // return seq_no=0, inserting=true
  return action(0u, 'I');
}


// When we find ^AE.
const char *
seq_state::end(seq_no seq)
{
  list<action>::iterator i = active_actions.begin();
  while (i != active_actions.end())
    {
      if (i->seq == seq)
	{
	  active_actions.erase(i);
	  decide_disposition();
#ifdef DEBUG_COMMANDS
	  fprintf(stderr,
		  "^A%c %u: inserting=%c\n",
		  'E',
		  (unsigned) seq,
		  inserting ? 'Y' : 'N');
#endif	  
	  return NULL;	      
	}
      ++i;
    }
  // if we get to here, the specified seq was not already active.
  return "unmatched ^AE";
}


// Tells us if the delta at the top of the stack is being included.
int
seq_state::include_line() const
{
  return inserting;
}

seq_no seq_state::active_seq() const
{
  return active;
}

#endif
