/*
 * sf-get2.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999,2001,2002,2007,2008 Free Software Foundation, Inc. 
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of the class sccs_file only used by get.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"
#include "delta-iterator.h"
#include "delta-table.h"


#include <ctype.h>

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
      if ( (release)found > (release)requested )
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
          requested = (release)delta_table->highest_release();

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
  
  const_delta_iterator iter(delta_table);
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
      && requested <= delta_table->highest_seqno()
      && delta_table->delta_at_seq_exists(requested))
    {
      found = delta_table->delta_at_seq(requested).id();
      return true;
    }
  else
    {
      return false;
    }
}


bool sccs_file::sid_in_use(sid id, sccs_pfile &pfile) const
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
                         sccs_pfile &pfile,
                         int *pfailed) const
{
  if (!flags.branch)
    want_branch = false;        // branches not allowed!
  
  if (flags.default_sid)
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
                          name.c_str());
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
                          name.c_str());
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
                          name.c_str());
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
	       name.c_str());
      return false;
    }

  sccs_pfile::const_iterator it = pf.find_locked(got);
  if (it != pf.end())
    {
      if (!flags.joint_edit)
	{
	  mystring when(it->date.as_string());
	  errormsg("%s: Requested SID locked by '%s' at %s.\n",
		   name.c_str(),
		   it->user.c_str(),
		   when.c_str());
	  return false;
	}
    }
  return true;
}


/* Output the specified version to a file with possible modifications.
   Most of the actual work is done with a seqstate object that 
   figures out whether or not given line of the SCCS file body
   should be included in the output file. */
        
/* struct */ sccs_file::get_status
sccs_file::get(FILE *out, mystring gname,
	       FILE *summary_file,
	       sid id, sccs_date cutoff_date,
               sid_list include, sid_list exclude,
               int keywords, const char *wstring,
               int show_sid, int show_module, int debug,
	       bool for_edit)
{
  
  /* Set the return status. */
  struct get_status status;
  status.success = true;
  
  ASSERT(0 != delta_table);
  
  seq_state state(highest_delta_seqno());
  const delta *d = find_delta(id);
  ASSERT(d != NULL);
  
  ASSERT(0 != delta_table);

  if (!edit_mode_ok(for_edit))	// "get -e" on BK files is not allowed
    {
      status.success = false;
      return status;
    }
  
  
  if (!prepare_seqstate(state, d->seq(),
                        include, exclude, cutoff_date))
    {
      status.success = false;
      return status;
    }

  // Fix by Mark Fortescue.  
  // Fix Cutoff Date Problem
  const delta *dparm;
  bool set=false;

  for (seq_no s = d->seq(); s>0; s--)
    {
      if (delta_table->delta_at_seq_exists(s))
	{
	  const struct delta & d = delta_table->delta_at_seq(s);
	  const sid & id(d.id());
	  
	  if (!state.is_excluded(s) && !set)
	    {
	      dparm = find_delta(id);
	      set = true;
	    }
	}
    }
  if ( !set ) dparm = d;
  // End of fix

  if (getenv("CSSC_SHOW_SEQSTATE"))
    {
      for (seq_no s = d->seq(); s>0; s--)
        {
          if (!delta_table->delta_at_seq_exists(s))
            {
              /* skip non-existent seq number */
              continue;
            }

          const struct delta & d = delta_table->delta_at_seq(s);
          const sid & id(d.id());

          fprintf(stderr, "%4d (", s);
          id.dprint(stderr);
          fprintf(stderr, ") ");

          if (state.is_explicitly_tagged(s))
            {
              fprintf(stderr, "explicitly ");
            }
          
          if (state.is_ignored(s))
            {
              fprintf(stderr, "ignored  by %4d\n",
                      state.whodunit(s));
            }
          else if (state.is_included(s))
            {
              fprintf(stderr, "included by %4d\n",
                      state.whodunit(s));
            }
          else if (state.is_excluded(s))
            {
              fprintf(stderr, "excluded by %d\n",
                      state.whodunit(s));
            }
          else
            {
              fprintf(stderr, "irrelevant\n");
            }
        }
    }

  if (summary_file)
    {
      bool first = true;
      
      for (seq_no s = d->seq(); s>0; s--)
        {
          if (delta_table->delta_at_seq_exists(s)
	      && state.is_included(s))
	    {
	      const struct delta & it = delta_table->delta_at_seq(s);
	      
	      fprintf (summary_file, "%s   ",
		       first ? "" : "\n");
	      first = false;
	      it.id().print(summary_file);
	      fprintf (summary_file, "\t");
	      it.date().print(summary_file);
	      fprintf (summary_file, " %s\n", it.user().c_str());
	      
	      if (it.comments().length())
		{
		  const int len = it.comments().length();
		  for(int i = 0; i < len; i++)
		    {
		      fprintf (summary_file, "\t%s\n",
			       it.comments()[i].c_str());
		    }
		}
	    }
	}
    }
  
  
  
  // The subst_parms here may not be the Whole Truth since
  // the cutoff date may affect which version is actually
  // gotten.  That's taken care of; the correct delta is
  // passed as a parameter to the substitution function.
  // (eugh...)
  // Changed to use dparm not d to deal with Cutoff Date (Mark Fortescue)
  struct subst_parms parms(out, wstring, *dparm,
                           0, sccs_date::now());
  
  
  status.success = get(gname, state, parms, keywords,
                       show_sid, show_module, debug);

  if (status.success == false)
    {
      return status;
    }
  // only issue a warning about there being no keywords
  // substituted, IF keyword substitution was being done.
  if (keywords && !parms.found_id)
    {
      no_id_keywords(name.c_str());
      // this function normally returns.
    }
  
  status.lines = parms.out_lineno;
  
  seq_no seq;   
  for(seq = 1; seq <= highest_delta_seqno(); seq++)
    {
      if (state.is_explicitly_tagged(seq))
        {
          const sid id = seq_to_sid(seq);

          if (state.is_included(seq))
            status.included.add(id);
          else if (state.is_excluded(seq))
            status.excluded.add(id);
        }
    }
  
  return status;
}

/* Local variables: */
/* mode: c++ */
/* End: */
