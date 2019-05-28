/*
 * sf-rmdel.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2001, 2007, 2008, 2009, 2010, 2011, 2014,
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Members of the class sccs_file used for marking a delta in the SCCS
 * files as removed.
 *
 */

#include <config.h>
#include <algorithm>
#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-iterator.h"
#include "linebuf.h"

namespace {
bool
is_seqlist_member(seq_no seq_to_find, std::vector<seq_no> const &seq_list) {
  return std::find(seq_list.begin(), seq_list.end(), seq_to_find) != seq_list.end();
}
}

bool
sccs_file::rmdel(sid id)
{
  if (!edit_mode_permitted(true).ok())
    return false;

  delta *d = find_delta(id);
  if (0 == d)
    {
      errormsg("%s: Specified SID not found in SCCS file.", name.c_str());
      return false;
    }
  const seq_no seq = d->seq();

  const_delta_iterator iter(delta_table.get(), delta_selector::current);
  while (iter.next())
    {
      if (iter->prev_seq() == seq)
	{
	  errormsg("%s: Specified SID has a successor.", name.c_str());
	  return false;
	}
      if (is_seqlist_member(seq, iter->get_included_seqnos())
	  || is_seqlist_member(seq, iter->get_excluded_seqnos())
	  || is_seqlist_member(seq, iter->get_ignored_seqnos()))
	{
	  errormsg("%s: Specified SID is used in another delta.",
	       name.c_str());
	  return false;
	}
    }

  d->set_type('R');

  FILE *out = start_update();
  if (NULL == out)
    return false;

  if (write(out))
    {
      xfile_error("Write error.");
      return false;
    }

  if (body_scanner_->remove(out, seq))
    {
      // Only finish write out the file if we had no problem.
      return end_update(&out);
    }
  return false;
}

/* Local variables: */
/* mode: c++ */
/* End: */
