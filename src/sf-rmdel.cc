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
#include "failure.h"
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

cssc::Failure
sccs_file::rmdel(sid id)
{
  cssc::Failure can_edit = edit_mode_permitted(true);
  if (!can_edit.ok())
    return can_edit;

  delta *d = find_delta(id);
  if (nullptr == d)
    {
      return cssc::make_failure_builder(cssc::errorcode::UsagePreconditionFailureSidNotFound)
	.diagnose()
	<< "Revision " << id << " is not present in " << name_.sfile();
    }
  const seq_no seq = d->seq();

  const_delta_iterator iter(delta_table_.get(), delta_selector::current);
  while (iter.next())
    {
      if (iter->prev_seq() == seq)
	{
	  auto prev_sid = iter->id();
	  return cssc::make_failure_builder(cssc::errorcode::UsagePreconditionFailureDeltaHasSuccessor)
	    .diagnose()
	    << "Revision " << id << " has a successor " << prev_sid
	    << " in " << name_.sfile();
	}
      if (is_seqlist_member(seq, iter->get_included_seqnos())
	  || is_seqlist_member(seq, iter->get_excluded_seqnos())
	  || is_seqlist_member(seq, iter->get_ignored_seqnos()))
	{
	  return cssc::make_failure_builder(cssc::errorcode::UsagePreconditionFailureDeltaInUse)
	    .diagnose()
	    << "Revision " << id << " is used by " << iter->id() << " in " << name_.sfile();
	}
    }

  d->set_type('R');

  cssc::FailureOr<FILE*> fof = start_update();
  if (!fof.ok())
    return fof.fail();
  ASSERT(*fof != NULL);
  FILE *out = *fof;

  cssc::Failure written = write(out);
  if (!written.ok())
    return written;

  cssc::Failure removed = body_scanner_->remove(out, seq);
  if (!removed.ok())
    return removed;

  // Only finish write out the file if we had no problem.
  cssc::Failure updated = end_update(&out);
  if (!updated.ok())
    {
      return cssc::make_failure_builder(updated)
	.diagnose() << "failed to complete update";
    }
  ASSERT(out == NULL);
  return cssc::Failure::Ok();
}

/* Local variables: */
/* mode: c++ */
/* End: */
