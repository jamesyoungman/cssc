/*
 * sf-get2.c: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-get2.cc,v 1.19 1997/11/15 20:06:10 james Exp $";
#endif

/* Returns the SID of the delta to retrieve that best matches the
   requested SID. */

#if 0
bool
sccs_file::find_requested_sid(sid requested, sid &found) const
{
  if (requested.is_null())
    {
      requested = flags.default_sid;
    }
  
  sid best;
  bool got_best = false;

  /* Find the delta with the highest SID that matches the
     requested SID */

  delta_iterator iter(delta_table);

  /* if requested.is_null(), this means that no SID was specified
   * on the command line and no default SID is set in the file.
   * Hence we return the highest release and level on the trunk.
   */
  if (requested.is_null())
    {
      best = sid("1.1");	// First SID on the trunk.
      while(iter.next())
	{
	  sid const &id = iter->id;
	  if ((best == id) || best.is_trunk_successor(id))
	    {
	      // If ID is on the trunk and is either after BEST, or
	      // the same as BEST, we have a new BEST.
	      best = id;
	      got_best = true;
	    }
	}
      assert(got_best);
      found = best;
      return true;
    }
  else				/* A SID was specified on the command line. */
    {
      while(iter.next())
	{
	  sid const &id = iter->id;
	  if (id == requested)
	    {
	      /* Found an exact match. */
	      found = requested;
	      return true;
	    }
	  else if ( requested.partial_match(id) )
	    {
	      if (best.is_null() || id > best )
		{
		  best = id;
		  got_best = true;
		}
	    }
	}
      if ( got_best )
	{
	  assert(!best.is_null());
	  found = best;
	  return true;
	}
      else if ( !requested.release_only() )
	{
	  found = best;
	  return false;
	}

      /* If a match wasn't found above and the requested SID
	 only mentions a release then look for the delta with 
	 the highest SID that's lower than the requested SID. */

      sid root("1.1");
      best = sid();
      got_best = false;
	
      iter.rewind();
      while(iter.next())
	{
	  sid const &id = iter->id;
	  if (!got_best && (root.is_trunk_successor(id) || root == id) )
	    {
	      got_best = true;
	      best = id;
	    }
	  else if (id > best && id < requested)
	    {
	      got_best = true;
	      best = id;
	    }
	}
      if (got_best)
	{
	  found = best;
	  return true;
	}
      else
	{
	  return false;		// TODO: what???
	}
    }
	
}
#else
bool
sccs_file::find_requested_sid(sid requested, sid &found, bool include_branches) const
{
  if (requested.is_null())	// no sid specified?
    {				// get the default.
      requested = flags.default_sid; 
      if (requested.is_null())	// no default?
	{			// get the latest.
	  requested = sid(release::LARGEST);
	}
    }

  // Giving a SID of two components is a request for 
  // an exact match on the trunk, unless include_branches
  // is specified, in which case it is a request for
  // the latest SID of the specified release and level.
  int ncomponents = requested.components();
  if (2 == ncomponents && !include_branches)
    ncomponents = 4;		// want an exact match.

  assert(ncomponents != 0);
  assert(ncomponents <= 4);

  // Remember the best so far.
  bool got_best = false;
  sid best;
  
  delta_iterator iter(delta_table);

  if (1 == ncomponents)
    {
      // find highest SID of any level, which is less than or equal to
      // the requested one.  If include_branches is true, they don't
      // have to be on the trunk.
      while(iter.next())
	{
	  if ( (release)iter->id > (release)requested )
	    continue;
	  else if (!include_branches && !iter->id.on_trunk())
	    continue;
	  if (!got_best || iter->id.gte(best))
	    {
	      best = iter->id;
	      got_best = true;
	    }
	}
    }
  else
    {
      while(iter.next())
	{
	  if (iter->id.matches(requested, ncomponents))
	    {
	      if (!got_best || iter->id.gte(best))
		{
		  best = iter->id;
		  got_best = true;
		}
	    }
	}
    }

  if (got_best)
    found = best;
  return got_best;
}

#endif

bool sccs_file::sid_in_use(sid id, sccs_pfile &pfile) const
{
  if (delta_table.find(id))
    return true;

  if (pfile.is_to_be_created(id))
    return true;

  return false;
}


/* Returns the SID of the new delta to be created. */
#if 0
sid
sccs_file::find_next_sid(sid requested, sid got, int branch,
			 sccs_pfile &pfile) const
{
  branch = branch && flags.branch;

  if (!branch)
    {
      /* Not sure what the point of the test whose result is succ is.
       */
      int succ = got.is_trunk_successor(delta_table.highest_release());
      if (!succ)
	{
	  if (requested > delta_table.highest_release())
	    {
	      assert(requested.release_only());
	      return requested.successor();
	    }
		
	  sid next = got.successor();
		
	  if (!pfile.is_to_be_created(next) && delta_table.find(next) == NULL)
	    {
	      return next;
	    }
	}
    }
	
  sid highest = got;

  delta_iterator iter(delta_table);
  while(iter.next())
    {
      sid const &id = iter->id;

      if (id.branch_greater_than(highest))
	{
	  highest = id;
	}
    }
	
  pfile.rewind();
  while(pfile.next())
    {
      sid const &id = pfile->delta;
		
      if (id.branch_greater_than(highest))
	{
	  highest = id;
	}
    }
	
  return highest.next_branch();
}
#else
sid
sccs_file::find_next_sid(sid requested, sid got,
			 int want_branch,
			 sccs_pfile &pfile) const
{
  if (!flags.branch)
    want_branch = false;	// branches not allowed!
  
  const int ncomponents = requested.components();
  bool forced_branch = false;

  sid next = requested;
  if (next.release_only())
    next.next_level();
  else
    ++next;
  
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
      const bool branch_again
	= delta_table.find(next) && !requested.partial_sid();
      
      if (too_high || branch_again)
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
      while (delta_table.find(next)
	     || (flags.joint_edit && pfile.is_to_be_created(next)))
	{
	  next.next_branch();
	}
    }

  assert(!sid_in_use(next, pfile));
  return next;
}

#endif

/* Quits if the user isn't authorized to make deltas, if the release
   requested is locked or if the requested SID has an edit lock and
   the joint edit flag isn't set. */

void
sccs_file::test_locks(sid got, sccs_pfile &pfile) const {
	int i;
	int len;

	const char *user = get_user_name();

	len = users.length();
	if (len != 0) {
		int found = 0;

		for(i = 0; i < len; i++) {
			const char *s = users[i];
			char c = s[0];

			if (c == '!') {
				s++;
				if (isdigit(c)) {
					if (user_is_group_member(atoi(s))) {
						found = 0;
						break;
					}
				} else if (strcmp(s, user) == 0) {
					found = 0;
					break;
				}
			} else {
				if (isdigit(c)) {
					if (user_is_group_member(atoi(s))) {
						found = 1;
					}
				} else if (strcmp(s, user) == 0) {
					found = 1;
					break;
				}
			}
		}
		
		if (!found) {
			quit(-1, "%s: You are not authorized to make deltas.",
			     (const char *) name);
		}
	}

	if (flags.all_locked 
	    || (flags.floor.valid() && flags.floor > got)
	    || (flags.ceiling.valid() && flags.ceiling < got)
	    || flags.locked.member(got)) {
		quit(-1, "%s: Requested release is locked.",
		     (const char *) name);
	}
	
	if (pfile.is_locked(got) && !flags.joint_edit) {
		quit(-1, "%s: Requested SID locked by '%s'.\n",
		     (const char *) name,
		     (const char *) pfile->user);
	}
}


/* Write a line of a file after substituting any id keywords in it.
   Returns true if an error occurs. */

int
sccs_file::write_subst(const char *start,
		       struct subst_parms *parms,
		       struct delta const& delta) const {
  //	struct delta const &delta = parms->delta;
	FILE *out = parms->out;

	const char *percent = strchr(start, '%');
	while(percent != NULL) {
		char c = percent[1];
		if (c != '\0' && percent[2] == '%') {
			if (start != percent
			    && fwrite(start, percent - start, 1, out) != 1) {
				return 1;
			}

			percent += 3;

			int err = 0;

			switch(c) {
				const char *s;

			case 'M':
			{
				mystring module = get_module_name();
				err = fputs_failed(fputs(module, out));
			}
				break;
			
			case 'I':
				err = delta.id.print(out);
				break;

			case 'R':
				err = delta.id.printf(out, 'R', 1);
				break;

			case 'L':
				err = delta.id.printf(out, 'L', 1);
				break;

			case 'B':
				err = delta.id.printf(out, 'B', 1);
				break;

			case 'S':
				err = delta.id.printf(out, 'S', 1);
				break;

			case 'D':
				err = parms->now.printf(out, 'D');
				break;
				
			case 'H':
				err = parms->now.printf(out, 'H');
				break;

			case 'T':
				err = parms->now.printf(out, 'T');
				break;

			case 'E':
				err = delta.date.printf(out, 'D');
				break;

			case 'G':
				err = delta.date.printf(out, 'H');
				break;

			case 'U':
				err = delta.date.printf(out, 'T');
				break;

			case 'Y':
				s = flags.type;
				if (s != NULL) {
					err = fputs_failed(fputs(s, out));
				}
				break;

			case 'F':
			  err = fputs_failed(fputs(base_part(name), out));
				break;

			case 'P':
			  	if (1) // introduce new scope...
				  {
				    mystring path(canonify_filename(name));
				    err = fputs_failed(fputs(path, out));
				  }
				break;

			case 'Q':
				s = flags.user_def;
				if (s != NULL) {
					err = fputs_failed(fputs(s, out));
				}
				break;

			case 'C':
			  err = printf_failed(fprintf(out, "%d",
						      parms->out_lineno));
				break;

			case 'Z':
				if (fputc_failed(fputc('@', out))
				    || fputs_failed(fputs("(#)", out))) {
					err = 1;
				} else {
					err = 0;
				}
				break;

			case 'W':
				s = parms->wstring;
				if (s == NULL) {
					s = "%Z""%%M""%\t%""I%";
				} else {
					/* protect against recursion */
					parms->wstring = NULL; 
				}
				err = write_subst(s, parms, delta);
				if (parms->wstring == NULL) {
					parms->wstring = s;
				}
				break;

			case 'A':
				err = write_subst("%Z""%%Y""% %M""% %I"
						  "%%Z""%",
						  parms, delta);
				break;

			default:
				start = percent - 3;
				percent = percent - 1;
				continue;
			}

			parms->found_id = 1;

			if (err) {
				return 1;
			}
			start = percent;
		} else {
			percent++;
		}
		percent = strchr(percent, '%');
	}

	return fputs_failed(fputs(start, out));
}

/* Output the specified version to a file with possible modifications.
   Most of the actual work is done with a seqstate object that 
   figures out whether or not given line of the SCCS file body
   should be included in the output file. */
	
/* struct */ sccs_file::get_status
sccs_file::get(FILE *out, mystring gname, sid id, sccs_date cutoff_date,
	       sid_list include, sid_list exclude,
	       int keywords, const char *wstring,
	       int show_sid, int show_module, int debug) {

	seq_state state(delta_table.highest_seqno());
	struct delta const *delta = delta_table.find(id);
	assert(delta != NULL);
	prepare_seqstate(state, delta->seq);
	prepare_seqstate(state, include, exclude, cutoff_date);

	// The subst_parms here may not be the Whole Truth since
	// the cutoff date may affect which version is actually
	// gotten.  That's taken care of; the correct delta is
	// passed as a parameter to the substitution function.
	// (eugh...)
	struct subst_parms parms(out, wstring, *delta,
				 0, sccs_date::now());

#ifdef __GNUC__
    if (keywords)
        {
        get(gname, state, parms, &sccs_file::write_subst,
            show_sid, show_module, debug);
        }
    else
        {
        get(gname, state, parms, (subst_fn_t) 0,
            show_sid, show_module, debug);
        }
#else
    if (keywords)
        {
        get(gname, state, parms,
            (int (sccs_file::*)(const char *, struct subst_parms *) const) 0,
            show_sid, show_module, debug);
        }
    else
        {
        get(gname, state, parms, &sccs_file::write_subst,
	    show_sid, show_module, debug);
        }
#endif
    // only issue a warning about there being no keywords
    // substituted, IF keyword substitution was being done.
	if (keywords && !parms.found_id)
	  {
	    no_id_keywords(name);
	  }
				     
	/* Set the return status. */
	struct get_status status;

	status.lines = parms.out_lineno;
	
	seq_no seq;	
	for(seq = 1; seq <= delta_table.highest_seqno(); seq++) {
		if (state.is_explicit(seq)) {
			if (state.is_included(seq)) {
				status.included.add(delta_table[seq].id);
			} else if (state.is_excluded(seq)) {
				status.excluded.add(delta_table[seq].id);
			}
		}
	}
			
	return status;
}

/* Local variables: */
/* mode: c++ */
/* End: */
