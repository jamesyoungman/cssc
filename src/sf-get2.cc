/*
 * sf-get2.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2002, 2007, 2008, 2009, 2010,
 *  2011, 2014, 2019, 2024 Free Software Foundation, Inc.
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
 * Members of the class sccs_file only used by get.
 *
 */

#include <config.h>

#include <cstdlib>
#include <string>
#include <ctype.h>

#include "cssc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"
#include "delta-iterator.h"
#include "delta-table.h"
#include "subst-parms.h"


bool sccs_file::sid_matches(const sid& requested,
			    const sid& found,
			    bool get_top_delta) const
{
  // Giving a SID of two components is a request for
  // an exact match on the trunk, unless get_top_delta
  // is specified, in which case it is a request for
  // the latest SID of the specified release and level.

  int ncomponents = requested.components();
  if (2 == ncomponents && !get_top_delta)
    ncomponents = 4;            // want an exact match.

  // ASSERT(ncomponents != 0);
  ASSERT(ncomponents <= 4);

  if (1 == ncomponents)
    {
      if (found.get_release() > requested.get_release())
	return false;
      else if (!get_top_delta && !found.on_trunk())
	return false;
      else
	return true;
    }
  else
    {
      return found.matches(requested, ncomponents);
    }
}



/* Returns the SID of the delta to retrieve that best matches the
   requested SID. */
bool
sccs_file::find_requested_sid(sid requested, sid &found, bool get_top_delta) const
{
  if (requested.is_null())      // no sid specified?
    {                           // get the default.
      requested = flags.default_sid;
      if (requested.is_null())  // no default?
        {                       // get the latest.
          requested = release(delta_table_->highest_release());

          if (!get_top_delta && requested.is_null())
          {
              return false; // no latest on the trunk.(SF bug 664900)
          }
        }
    }

  // Giving a SID of two components is a request for
  // an exact match on the trunk, unless get_top_delta
  // is specified, in which case it is a request for
  // the latest SID of the specified release and level.
  int ncomponents = requested.components();
  if (2 == ncomponents && !get_top_delta)
    ncomponents = 4;            // want an exact match.

  // ASSERT(ncomponents != 0);
  ASSERT(ncomponents <= 4);

  // Remember the best so far.
  bool got_best = false;
  sid best;

  // find highest SID of any level, which is less than or equal to
  // the requested one.
  //
  // If get_top_delta (which corresponds to the -t
  // option of "get"), this means that the user wants
  // to find the "top" * delta - this is the one most
  // recently added to the SCCS file, that is the one
  // closest to the beginning of the file.  It is not
  // possible to determine which SID this is by
  // looking at the tree of SIDs alone.

  const_delta_iterator iter(delta_table_.get(), delta_selector::current);
  while (iter.next())
    {
      if (sid_matches(requested, iter->id(), get_top_delta))
	{
	  if (!got_best || iter->id().gte(best))
	    {
	      best = iter->id();
	      got_best = true;

	      if (get_top_delta
		  && iter->id().matches(requested, ncomponents))
		{
		  break;
		}
	    }
	}
    }


  if (got_best)
    found = best;
  return got_best;
}



bool
sccs_file::find_requested_seqno(seq_no requested, sid &found) const
{
  if (requested > 0
      && requested <= delta_table_->highest_seqno()
      && delta_table_->delta_at_seq_exists(requested))
    {
      found = delta_table_->delta_at_seq(requested).id();
      return true;
    }
  else
    {
      return false;
    }
}


bool sccs_file::sid_in_use(sid id, const sccs_pfile &pfile) const
{
  if (find_delta(id))
    return true;

  sccs_pfile::const_iterator i = pfile.find_to_be_created(id);
  if (i == pfile.end())
    return false;
  else
    return true;

}


/* Returns the SID of the new delta to be created. */
sid
sccs_file::find_next_sid(sid requested, sid got,
                         int want_branch,
                         const sccs_pfile &pfile,
                         int *pfailed) const
{
  if (!flags.branch)
    want_branch = false;        // branches not allowed!

  if (!flags.default_sid.is_null())
    {
      requested = flags.default_sid;
    }

  const int ncomponents = requested.components();
  bool forced_branch = false;

  sid next = requested;
  if (requested.release_only())
    {
      next.next_level();

      // JY: 2003-12-07: test e6 of defsid.sh was failing.
      while (sid_in_use(next, pfile))
	next.next_level();
    }
  else if (requested.partial_sid())
    {
      next = got;
      ++next;
    }
  else
    {
      ++next;
    }

  if (want_branch)
    {
      if (ncomponents < 4)
        next = got;

      next.next_branch();
    }
  else
    {
      // We may be forced to create a branch anyway.

      // have we hit the release ceiling?
      const bool too_high = requested.on_trunk()
        && flags.ceiling.valid() && release(requested) > flags.ceiling;

      // have we collided with an existing SID?
      bool branch_again;
      const delta *pnext = find_any_delta(next);
      if (pnext)
        {
          if (!pnext->removed() && !requested.partial_sid())
            {
              branch_again = true;
            }
          else
            {
	      // SourceForge bug 865422: if pnext is listed in the
	      // pfile, it counts as being in use.
	      if (sid_in_use(next, pfile) && flags.joint_edit)
		{
                  warning("%s: creating a branch "
                          "due to concurrent edit",
                          name_.c_str());
		  branch_again = true;
		}
	      else
		{
		  branch_again = false;
		}
            }
        }
      else
        {
          /* Whew, the SID is not already used in the SCCS file;
           * check the p-file also though...
           */
          if (sid_in_use(next, pfile))
            {
              if (flags.joint_edit)
                {
                  warning("%s: creating a branch "
                          "due to concurrent edit",
                          name_.c_str());
                  branch_again = true;
                }
              else
                {
                  /* If the requested SID is already being edited,
                   * and the joint edit flag is not set, I think that
                   * the attempt to edit the file shpuld already have been
                   * thrown out by sccs_file::test_locks().
                   */
                  warning("%s: requested SID is "
                          "already being edited; this should not happen",
                          name_.c_str());
                  *pfailed = 1;
                  return next; // FAILURE
                }
            }
          else
            {
              branch_again = false;
            }
        }


      // If we have the revision sequence 1.1 -> 1.2 -> 2.1, then we
      // get 1.2 for editing, we must create a branch (1.2.1.1),
      // because we can't create a 1.3 (as 2.1 already exists).  If
      // the release number of the gotten SID is not the highest, we
      // have to branch.  Otherwise I think the normal anti-collision
      // rules take care of it.
      bool not_trunk_top;
      if (ncomponents < 3)
        {
          not_trunk_top = release(got) < release(highest_delta_release());
        }
      else
        {
          /* If 4 components were specified, then we don't care if the
           * current release is not the highest release.  If we
           * specified that we want to check 1.2.1.1 out for editing
           * and in fact 1.2.1.2 alredy exists, we should just fail,
           * rather than making a branch.
           */
          not_trunk_top = false;
        }
      if (too_high || branch_again || not_trunk_top)
        {
          next = got;
          next.next_branch();
          forced_branch = true;
        }
    }

  // If we have created a branch, and that branch is not unique, keep
  // looking for an empty branch.
  if (want_branch || forced_branch)
    {
      while (find_delta(next)
             || (flags.joint_edit && pfile.is_to_be_created(next)))
        {
          next.next_branch();
        }
    }

  ASSERT(!sid_in_use(next, pfile));
  return next;
}

/* Quits if the user isn't authorized to make deltas, if the release
   requested is locked or if the requested SID has an edit lock and
   the joint edit flag isn't set. */

bool
sccs_file::test_locks(sid got, const sccs_pfile& pf) const
{
  if (!authorised())
    return false;

  if (flags.all_locked
      || (flags.floor.valid() && flags.floor > got)
      || (flags.ceiling.valid() && flags.ceiling < got)
      || flags.locked.member(got))
    {
      errormsg("%s: Requested release is locked.",
	       name_.c_str());
      return false;
    }

  sccs_pfile::const_iterator it = pf.find_locked(got);
  if (it != pf.end())
    {
      if (!flags.joint_edit)
	{
	  std::string datestring(it->date.as_string());
	  errormsg("%s: Requested SID locked by '%s' at %s.\n",
		   name_.c_str(),
		   it->user.c_str(),
		   datestring.c_str());
	  return false;
	}
    }
  return true;
}


/* Local variables: */
/* mode: c++ */
/* End: */
