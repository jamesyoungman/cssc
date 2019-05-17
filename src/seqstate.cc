/*
 * seqstate.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Defines the non-inline members of the class seq_state.
 *
 */

#include <config.h>
#include <utility>
#include "cssc.h"
#include "seqstate.h"

#undef DEBUG_COMMANDS


seq_state::seq_state(seq_no l) :
  last_(l),
  active_(0u)
{
  states_.reserve(last_+1);
  one_state initial_condition;
  initial_condition.included = false;
  initial_condition.ignored = false;
  initial_condition.excluded = false;
  initial_condition.is_explicit = false;
  initial_condition.done_by = 0u;
  initial_condition.non_recursive = false;
  initial_condition.active = false;
  initial_condition.command = '\0';
  for (decltype(last_) i = 0; i <= last_; ++i)
    {
      states_.push_back(initial_condition);
    }
  decide_disposition();
}


seq_state::seq_state(const seq_state& s) :
  last_(s.last_),
  active_(s.active_)
{
  states_ = s.states_;
  decide_disposition();
}



bool seq_state::is_included(seq_no n) const
{
  return states_[n].included;
}

bool seq_state::is_excluded(seq_no n) const
{
  return states_[n].excluded;
}

bool seq_state::is_ignored(seq_no n) const
{
  return states_[n].ignored;
}

void seq_state::set_explicitly_included(seq_no n, seq_no who)
{
  if (!states_[n].included)  	// if not already included...
    {
      set_included(n, who);
      one_state& s = states_[n];
      s.is_explicit = true;
      s.non_recursive = true;
      s.done_by = who;
    }
}

void seq_state::set_explicitly_excluded(seq_no n, seq_no who)
{
  set_excluded(n, who);
  one_state& s = states_[n];
  s.is_explicit = true;
  s.non_recursive = true;
  s.done_by = who;
}

void seq_state::set_included(seq_no n,
			     seq_no who,
			     bool bNonRecursive /*=false*/)
{
  one_state& s = states_[n];
  s.included = true;
  s.ignored = false;
  s.excluded = false;
  s.non_recursive = bNonRecursive;
  s.done_by = who;
}

void seq_state::set_ignored(seq_no n, seq_no who)
{
  one_state& s = states_[n];
  s.ignored = true;
  s.included = false;
  s.excluded = false;
  s.non_recursive = true;
  s.done_by = who;
}

void seq_state::set_excluded(seq_no n, seq_no who)
{
  one_state& s = states_[n];
  s.excluded = true;
  s.included = false;
  s.ignored = false;
  s.done_by = who;
}

bool seq_state::is_explicitly_tagged(seq_no n) const
{
  return states_[n].is_explicit;
}

bool seq_state::is_nonrecursive(seq_no n) const
{
  return states_[n].non_recursive;
}

bool seq_state::is_recursive(seq_no n) const
{
  return !states_[n].non_recursive;
}

seq_no seq_state::whodunit(seq_no n) const
{
  return states_[n].done_by;
}


seq_state::~seq_state()
{
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

  for (seq_no s=0; s <= last_; ++s)
    {
      const one_state& current = states_[s];
      if (!current.active)
	{
	  continue;
	}

      if ('I' == current.command)
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
      // Our deletion supercedes insertion
      inserting = false;
    }
  else if (our_highest_insert > owner_of_current_insertion)
    {
      bool ignored = false;

      active_ = our_highest_insert;
      inserting = true;
      if (is_ignored(active_))
	ignored = true;

      if (ignored)
	{
	  // Our insertion is in scope and is unsuperceded but is ignored
	  inserting = false;
	}
      else
	{
	  // Our insertion is in scope and is unsuperceded
	  inserting = true;
	}
    }
  else
    {
      // We have no unsuperceded insertion in scope
      inserting = false;
    }
}


// When we find ^AI or ^AD
std::pair<bool,std::string>
seq_state::start(seq_no seq, char command_letter)
{
  auto fail = [](const char *msg) -> std::pair<bool, std::string>
    {
      return std::make_pair(false, std::string(msg));
    };

  // begin diagnostic-only code.
  if (command_letter != 'I' && command_letter != 'D')
    {
      return fail("invalid command letter");
    }
  else if (seq > last_)
    {
      return fail("invalid sequence number");
    }
  else if (states_[seq].active)
    {
      if (states_[seq].command == 'I')
	{
	  return fail("^AI for sequence number which is already active");
	}
      else
	{
	  return fail("^AD for sequence number which is already active");
	}
    }
  // end diagnostic-only code.


  states_[seq].active = true;
  states_[seq].command = command_letter;
  decide_disposition();

#ifdef DEBUG_COMMANDS
  fprintf(stderr,
	  "^A%c %u: inserting=%c \n",
	  command_letter,
	  (unsigned) seq,
	  inserting ? 'Y' : 'N');
#endif
  return std::make_pair(true, std::string());;
}

// When we find ^AE.
std::pair<bool, std::string>
seq_state::end(seq_no seq)
{
  auto fail = [](const char *msg) -> std::pair<bool, std::string>
    {
      return std::make_pair(false, std::string(msg));
    };
  if (seq > last_)
    {
      return fail("invalid sequence number");
    }
  else if (states_[seq].active)
    {
      states_[seq].active = false;
      states_[seq].command = 0;
      decide_disposition();
#ifdef DEBUG_COMMANDS
      fprintf(stderr,
	      "^A%c %u: inserting=%c\n",
	      'E',
	      (unsigned) seq,
	      inserting ? 'Y' : 'N');
#endif
      return std::make_pair(true, std::string());;
    }
  else
    {
      return fail("unmatched ^AE");
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
  return active_;
}
