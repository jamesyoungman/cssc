/*
 * sf-get3.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2002, 2007, 2008, 2009, 2010, 2011, 2014,
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
 * Members of the class sccs_file used by get and delta.
 *
 */

#include <config.h>

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta-iterator.h"
#include "file.h"

/* Prepare a seqstate for use by marking which sequence numbers are to
 * be included and which are to be excluded.
 */

void
sccs_file::prepare_seqstate_2(seq_state &state, sid_list include,
			      sid_list exclude, sccs_date cutoff_date)
{

  ASSERT(nullptr != delta_table_);
  const_delta_iterator iter(delta_table_.get(), delta_selector::current);

  while (iter.next())
    {
      sid const &id = iter->id();

      if (include.member(id))   // explicitly included on command line
        {
          state.set_explicitly_included(iter->seq());
        }
      else if (exclude.member(id)) // explicitly included on command line
        {
          state.set_explicitly_excluded(iter->seq());
        }
      else if (cutoff_date.valid() && iter->date() > cutoff_date)
        {
          // Delta not explicitly included/excluded by the user, but
          // if it is newer than the cutoff date, we don't want it.
          // This is the feature that allows us to retrieve a delta
          // that was current at some time in the past.
          state.set_excluded(iter->seq());
        }

      ASSERT(nullptr != delta_table_);
    }
  ASSERT(nullptr != delta_table_);
}


void sccs_file::prepare_seqstate(seq_state &state, seq_no seq,
                                 sid_list include,
                                 sid_list exclude, sccs_date cutoff_date)
{
    prepare_seqstate_1(state, seq);
    prepare_seqstate_2(state, include, exclude, cutoff_date);
}




bool
sccs_file::authorised() const
{
  if (users_.empty())
    {
      // If there is no list of authorized users, all users are authorised.
      return true;
    }

  const char *myself = get_user_name();
  for (const auto& authorized_user : users_)
    {
      if (isdigit(authorized_user[0]))
	{
	  // FIXME: don't use atoi, it doesn't do error detection well.
	  if (user_is_group_member(atoi(authorized_user.c_str())))
	    {
	      return true;
	    }
	}
      else if (myself == authorized_user)
	{
	  return true;
	}
    }
  errormsg("%s: You are not authorized to make deltas.", name_.c_str());
  return false;
}


/* Local variables: */
/* mode: c++ */
/* End: */
