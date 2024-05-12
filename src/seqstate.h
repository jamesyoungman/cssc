/*
 * seqstate.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019, 2024 Free Software Foundation, Inc.
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the class seqstate.
 */

#ifndef CSSC__SEQSTATE_H__
#define CSSC__SEQSTATE_H__

#include "delta.h"

class cssc_delta_table;


/* This class is used to decide which lines of the body of a SCCS file
 * should be included in a gotten file.
 */


class seq_state
{
  // Make assignment and copy constructor private.
  const seq_state& operator=(const seq_state& s);

  struct one_state
  {
    bool included;
    bool excluded;
    bool ignored;
    bool non_recursive;
    bool is_explicit;
    bool active;
    char command;
  };

  std::vector<one_state> states_;
  seq_no          last_;
  seq_no          active_; // for use by "get -m" and so on.

  // We keep a record of the open ^AI or ^AD expressions
  // that are currently in effect, while reading the SCCS file.
  // This is kept in the pActive array.  If pActive[n] true,
  // then pCommand[i] contains an 'I' or 'D' indicating that a
  // ^AI or ^AD had been encountered.

  // TODO: rename member variables to consistently have a trailing "_".
  bool            inserting;	// current state.


  // Calculate a new value for the "inserting" flag.
  void decide_disposition();

public:
  seq_state(seq_no l);
  seq_state(const seq_state& s);
  ~seq_state();

  bool is_included(seq_no n) const;
  bool is_excluded(seq_no) const;
  bool is_ignored(seq_no) const;

  bool is_explicitly_tagged(seq_no) const;
  bool is_nonrecursive(seq_no n) const;
  bool is_recursive(seq_no n) const;

  void set_explicitly_included(seq_no which);
  void set_explicitly_excluded(seq_no which);
  void set_included(seq_no n, bool bNonRecursive=false);
  void set_excluded(seq_no n);
  void set_ignored (seq_no n);

  // stuff for use when reading the body of the s-file.

  // When we find ^AI or ^AD.
  // pair.first == success indicator
  // pair.second == error message
  // TODO: return cssc::Failure instead?
  std::pair<bool,std::string> start(seq_no seq, char command);

  // When we find ^AE.
  // TODO: return cssc::Failure instead?
  std::pair<bool,std::string> end(seq_no seq);

  // Tells us if the delta at the top of the stack is being included.
  int include_line() const;

  // finding out which seq is active, currently.
  seq_no active_seq() const;
};

#endif

/* Local variables: */
/* mode: c++ */
/* End: */
