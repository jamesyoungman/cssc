/*
 * sf-val.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,2001,2002,
 *                  2004,2007,2008 Free Software Foundation, Inc. 
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
 *
 * Members of class sccs_file used by "val". 
 * 
 */
#include <vector>
#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-table.h"
#include "delta-iterator.h"


namespace {

  // Check that there are no loops on the way from s to the first delta.
  bool check_loop_free(const char *name,
		       cssc_delta_table* t,
		       seq_no starting_seq,
		       std::vector<bool>& loopfree,
		       std::vector<bool>& seen)
  {
    bool ok = true;
    const seq_no highest_seqno = t->highest_seqno();
    seq_no lowest_dirty_seq = highest_seqno;

    std::vector<bool>::iterator i;

    // Check there are no loops.
    for (seq_no s=starting_seq;
	 s;
	 s = t->delta_at_seq(s).prev_seq())
      {
	if (s < lowest_dirty_seq)
	  lowest_dirty_seq = s;

	if (loopfree[s])
	  {
	    break;
	  }
	else if (seen[s])
	  {
	    errormsg("%s: loop in predecessors at sequence number %u\n",
		     name, (unsigned)s);
	    ok = false;
	    break;
	  }
	else
	  {
	    seen[s] = true;
	  }
      }

    // Mark the newly-explored loop-free graph as being loop-free.
    if (ok)
      {
	for (seq_no s=starting_seq;
	     s;
	     s = t->delta_at_seq(s).prev_seq())
	  {
	    if (loopfree[s])
	      break;
	    else if (seen[s])
	      loopfree[s] = true;
	  }
      }
    for (i = seen.begin()+lowest_dirty_seq; i != seen.end(); ++i)
      *i = false;

    return ok;
  }
}



const mystring
sccs_file::get_module_type_flag()
{
  if (flags.type)
    return *(flags.type);
  else
    return mystring("");
}


bool 
sccs_file::validate_seq_lists(const delta_iterator& d) const
{
  const char *sz_sid = d->id().as_string().c_str();
  mylist<seq_no>::size_type i;
  seq_no s;
  const seq_no highest_seq = delta_table->highest_seqno();
  
  for (i=0; i<d->get_included_seqnos().length(); ++i)
    {
      s = d->get_included_seqnos()[i];
      if (s > highest_seq)
	{
	  errormsg("%s: SID %s: included seqno %u does not exist\n",
		   name.c_str(), sz_sid, (unsigned)s);
	  return false;
	}
    }

  for (i=0; i<d->get_excluded_seqnos().length(); ++i)
     {
       s = d->get_excluded_seqnos()[i];
       if (s > highest_seq)
	 {
	   errormsg("%s: SID %s: excluded seqno %u does not exist\n",
		    name.c_str(), sz_sid, (unsigned)s);
	   return false;
	 }
     }

  for (i=0; i<d->get_ignored_seqnos().length(); ++i)
     {
       s = d->get_ignored_seqnos()[i];
       if (s > highest_seq)
	 {
	   errormsg("%s: SID %s: ignored seqno %u does not exist\n",
		    name.c_str(), sz_sid, (unsigned)s);
	   return false;
	 }
     }
   return true;
}

bool 
sccs_file::validate_isomorphism() const
{
  // TODO: write a bug-free version
  return true;
}

static bool
validate_substituted_flags_list(const mylist<char> entries)
{
  // TODO: write this later.
  return true;
}


bool 
sccs_file::validate() const
{
  bool retval = true;

  if (!checksum_ok())
    {
      return false;
    }

  // for each delta:-
  delta_iterator iter(delta_table);
  const seq_no highest_seq = delta_table->highest_seqno();
  int *seen_ever = new int[highest_seq];
  std::vector<bool> seen(highest_seq+1, false);
  std::vector<bool> loopfree(highest_seq+1, false);

  
  for (seq_no i=0; i<highest_seq; ++i)
    {
      seen_ever[i] = 0;
    }
  
  while ( retval && iter.next())
    {
      seq_no s = iter->seq();

      const char *sz_sid = iter->id().as_string().c_str();
      
      // validate that the included/excluded/unchanged line counts are valid.
      if (iter->inserted() > 99999uL)
	{
	  errormsg("%s: SID %s: out-of-range inserted line count %lu",
		   name.c_str(), sz_sid, iter->inserted());
	  retval = false;
	}
      if (iter->deleted() > 99999uL)
	{
	  errormsg("%s: SID %s: out-of-range deleted line count %lu",
		   name.c_str(), sz_sid, iter->deleted());
	  retval = false;
	}
      if (iter->unchanged() > 99999uL)
	{
	  errormsg("%s: SID %s: out-of-range unchanged line count %lu",
		   name.c_str(), sz_sid, iter->unchanged());
	  retval = false;
	}
      
      if (!iter->date().valid())
	{
	  errormsg("%s: SID %s: invalid date", name.c_str(), sz_sid);
	  retval = false;
	}
      
      // check that username contains no colon.
      if (iter->user().empty())
	{
	  errormsg("%s: SID %s: empty username", name.c_str(), sz_sid);
	  retval = false;
	}
      else if (iter->user().find_last_of(':') != mystring::npos)
	{
	  errormsg("%s: SID %s: invalid username '%s'",
		   name.c_str(), sz_sid, iter->user().c_str());
	  retval = false;
	}

      // check seqno is valid - loops, dangling references.
      if (s > highest_seq)
	{
	  errormsg("%s: SID %s: invalid seqno %d",
		   name.c_str(), sz_sid, (int)s);
	  retval = false;
	}
      
      if (iter->prev_seq() > highest_seq)
	{
	  errormsg("%s: SID %s: invalid predecessor seqno %d",
		   name.c_str(), sz_sid, (int)iter->prev_seq());
	  retval = false;
	}
      
      if (seen_ever[s - 1] > 1)
	{
	  errormsg("%s: seqno %d appears more than once (%d times)",
		   name.c_str(), (int)s, seen_ever[s - 1]);
	  retval = false;		// seqno appears more than once.
	}

      ++seen_ever[s - 1];

      if (!check_loop_free(name.c_str(), 
			   delta_table, delta_table->delta_at_seq(s).seq(),
			   loopfree, seen))
	{
	  // We already issued an error message.
	  retval = false;
	}
      
      // check time doesn't go backward (warning only, because this is
      // possible if developers in different timezones are
      // collaborating on the same file).
      if (0 != iter->prev_seq())
	{
	  const delta& ancestor(delta_table->delta_at_seq(iter->prev_seq()));
	  if (ancestor.date() > iter->date())
	    {
	      // Time has apparently gone backward...
	      warning("%s: date for version %s"
		       " is later than the date for version %s",
		       name.c_str(),
		      ancestor.id().as_string().c_str(),
		      iter->id().as_string().c_str());
	    }
	}
      
      // check included/excluded/ignored deltas actually exist.
      validate_seq_lists(iter);
    }
  delete[] seen_ever;
  
  if (false == retval)
    return retval;
  
  if (!validate_isomorphism())	// check that SIDs follow seqno structure.
    {
      return false;
    }
  
  // Check username list for invalid entries.
  for (mylist<mystring>::size_type j=0; j<users.length(); ++j)
    {
      const mystring& username(users[j]);
      
      if (   (username.find_last_of(':')  != mystring::npos) 
	  || (username.find_last_of(' ')  != mystring::npos)
	  || (username.find_last_of('\t') != mystring::npos))
	{
	  errormsg("%s: invalid username '%s'",
		   name.c_str(), username.c_str());
	  retval = false;
	}
    }
  
  
  // for the flags section:-

  // Check that the 'y' flag specifies only known keywords.
  const mylist<char> entries = flags.substitued_flag_letters.list();
  for (mylist<char>::size_type k=0; k<entries.length(); ++k)
    {
      char flag = entries[k];
      if (!is_known_keyword_char(flag))
	{
	  warning("The 'y' flag specifies a keyword letter '%c', "
		  "but %%%c%% is not a recognised SCCS keyword" ,
		  flag, flag);
	}
    }

  if (flags.floor > delta_table->highest_release())
    {
      warning("%s has a release floor of %d but the highest actual release "
	      "in the file is %d",
	      name.c_str(), (short)flags.floor,
	      delta_table->highest_release().as_string().c_str());
    }

  if (!flags.default_sid.is_null())
    {
      const delta* pd = delta_table->find_any(flags.default_sid);
      if (pd)
	{
	  if (pd->removed())
	    {
	      warning("%s has a default SID of %s, but that SID has "
		      "been removed",
		      name.c_str(), flags.default_sid.as_string().c_str());
	    }
	}
      else
	{
	  warning("%s has a default SID of %s, but that SID is not present",
		  name.c_str(), flags.default_sid.as_string().c_str());	      
	}
    }
  

  
  // TODO: check for unknown flags
  // TODO: check for boolean flags with non-numeric value.

  // TODO: check the body (unclosed deltas, etc.)
  
  return retval;
}


/* Local variables: */
/* mode: c++ */
/* End: */
