/*
 * seqstate.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999, Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * Defines the non-inline members of the class seq_state.
 *
 */

#include "cssc.h"
#include "seqstate.h"

#undef DEBUG_COMMANDS


seq_state::seq_state(seq_no l) :
  last(l),
  active(0u)
{
  pIncluded = new unsigned char[l + 1];
  pIgnored  = new unsigned char[l + 1];
  pExcluded = new unsigned char[l + 1];
  pExplicit = new unsigned char[l + 1];
  pDoneBy   = new seq_no       [l + 1];
  pNonrecursive = new unsigned char[l + 1];
  pActive   = new unsigned char[l + 1];
  pCommand  = new          char[l + 1];
  
  for(int i=0; i <= last; i++)
    {
      pIncluded[i] = 0;
      pIgnored[i]  = 0;
      pExcluded[i] = 0;
      pExplicit[i] = 0;
      pDoneBy[i]   = 0u;
      pNonrecursive[i] = 0;
      pActive[i]   = 0;
      pCommand[i]  = 0;
    }
  
  decide_disposition();
}


seq_state::seq_state(const seq_state& s) : 
  last(s.last),
  active(s.active)
  
{
  pIncluded = new unsigned char[last + 1];
  pIgnored  = new unsigned char[last + 1];
  pExcluded = new unsigned char[last + 1];
  pExplicit = new unsigned char[last + 1];
  pDoneBy   = new seq_no       [last + 1];
  pNonrecursive = new unsigned char[last + 1];
  pActive   = new unsigned char[last + 1];
  pCommand  = new          char[last + 1];
  
  for( int i=0; i <= last; i++)
    {
      pIncluded[i] = s.pIncluded[i];
      pIgnored [i] = s.pIgnored [i];
      pExcluded[i] = s.pExcluded[i];
      pExplicit[i] = s.pExplicit[i];
      pDoneBy  [i] = s.pDoneBy[i];
      pNonrecursive[i] = s.pNonrecursive[i];
      pActive[i]   = s.pActive[i];
      pCommand[i]  = s.pCommand[i];
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

bool seq_state::is_ignored(seq_no n) const
{
  return pIgnored[n];
}

void seq_state::set_explicitly_included(seq_no n, seq_no who)
{
  if (0 == pIncluded[n])	// if not already included...
    {
      set_included(n, who);
      pExplicit[n] = 1;
      pNonrecursive[n] = true;
      pDoneBy[n] = who;
    }
}

void seq_state::set_explicitly_excluded(seq_no n, seq_no who)
{
  set_excluded(n, who);
  pExplicit[n] = 1;
  pNonrecursive[n] = true;
  pDoneBy[n] = who;
}

void seq_state::set_included(seq_no n,
			     seq_no who,
			     bool bNonRecursive /*=false*/)
{
  pIncluded[n] = 1;
  pIgnored[n]  = 0;
  pExcluded[n] = 0;
  pNonrecursive[n] = bNonRecursive;
  pDoneBy[n] = who;
}

void seq_state::set_ignored(seq_no n, seq_no who)
{
  pIgnored [n] = 1;
  pIncluded[n] = 0;
  pExcluded[n] = 0;
  pNonrecursive[n] = true;
  pDoneBy[n] = who;
}

void seq_state::set_excluded(seq_no n, seq_no who)
{
  pExcluded[n] = 1;
  pIncluded[n] = 0;
  pIgnored [n] = 0;
  pDoneBy[n] = who;
}

bool seq_state::is_explicitly_tagged(seq_no n) const
{
  return pExplicit[n];
}

bool seq_state::is_nonrecursive(seq_no n) const
{
  return pNonrecursive[n];
}

bool seq_state::is_recursive(seq_no n) const
{
  return !is_nonrecursive(n);
}

seq_no seq_state::whodunit(seq_no n) const
{
  return pDoneBy[n];
}


seq_state::~seq_state()
{
  delete[] pIncluded;
  delete[] pIgnored;
  delete[] pExcluded;
  delete[] pExplicit;
  delete[] pDoneBy;
  delete[] pNonrecursive;
  delete[] pActive;
  delete[] pCommand;
  
  pIncluded = pExcluded = pIgnored = pExplicit = pNonrecursive = pActive = 0;
  pDoneBy = 0;
  pCommand = 0;
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
  
  for (seq_no s=0; s <= last; ++s)
    {
      if (!pActive[s])
	{
	  continue;
	}
      
      if ('I' == pCommand[s])
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
      bool ignored = false;
      
      active = our_highest_insert;
      inserting = true;
#if 0      
      for (seq_no s=0; s <= active; ++s)
	{
	  if (pActive[s] && is_ignored(s))
	    ignored = true;
	}
#else
      if (is_ignored(active))
	ignored = true;
#endif      
      if (ignored)
	{
#if 0
	  fprintf(stderr,
		  "Our insertion is in scope and is unsuperceded "
		  "but is ignored\n");
#endif
	  inserting = false;
	}
      else
	{
#if 0
	  fprintf(stderr, "Our insertion is in scope and is unsuperceded\n");
#endif
	  inserting = true;
	}
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
  else if (seq > last)
    {
      return "invalid sequence number";
    }
  else if (pActive[seq])
    {
      if (pCommand[seq] == 'I')
	{
	  return "^AI for sequence number which is already active";
	}
      else
	{
	  return "^AD for sequence number which is already active";
	}
    }
  // end diagnostic-only code.

       
  pActive[seq] = 1;
  pCommand[seq] = command_letter;
  
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

// When we find ^AE.
const char *
seq_state::end(seq_no seq)
{
  if (seq > last)
    {
      return "invalid sequence number";
    }
  else if (pActive[seq])
    {
      pActive[seq] = 0;
      pCommand[seq] = 0;
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
  else
    {
      return "unmatched ^AE";
    }
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
