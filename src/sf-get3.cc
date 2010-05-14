/*
 * sf-get3.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2002,2007 Free Software Foundation, Inc. 
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
 * Members of the class sccs_file used by get and delta.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta-iterator.h"
#include "file.h"

/* Prepare a seqstate for use by marking which sequence numbers are to
 * be included and which are to be excluded.
 */

bool
sccs_file::prepare_seqstate_2(seq_state &state, sid_list include,
                            sid_list exclude, sccs_date cutoff_date)
{
  
  ASSERT(0 != delta_table);
  const_delta_iterator iter(delta_table);

  while (iter.next())
    {
      sid const &id = iter->id();

      if (include.member(id))   // explicitly included on command line
        {
          state.set_explicitly_included(iter->seq(),
					(seq_no) seq_state::BY_COMMAND_LINE );
        }
      else if (exclude.member(id)) // explicitly included on command line
        {
          state.set_explicitly_excluded(iter->seq(),
					(seq_no) seq_state::BY_COMMAND_LINE );
        }
      else if (cutoff_date.valid() && iter->date() > cutoff_date)
        {
          // Delta not explicitly included/excluded by the user, but
          // if it is newer than the cutoff date, we don't want it.
          // This is the feature that allows us to retrieve a delta
          // that was current at some time in the past.
          state.set_excluded(iter->seq(), (seq_no) seq_state::BY_COMMAND_LINE);
        }
      
      ASSERT(0 != delta_table);
    }
  ASSERT(0 != delta_table);
  
  return true;
}


bool sccs_file::prepare_seqstate(seq_state &state, seq_no seq,
                                 sid_list include,
                                 sid_list exclude, sccs_date cutoff_date)
{
    if (!prepare_seqstate_1(state, seq))
        return false;
    
    if (!prepare_seqstate_2(state, include, exclude, cutoff_date))
        return false;
    
    return true;
}




bool
sccs_file::authorised() const {
  mylist<mystring>::size_type len, i;
  
  const char *user = get_user_name();
  
  len = users.length();
  if (len != 0) {
    int found = 0;
    
    for(i = 0; i < len; i++)
      {
        const char *s = users[i].c_str();
        char c = s[0];
      

	if (isdigit(c))
	  {
	    // FIXME: don't use atoi, it doesn't do error detection well.
	    if (user_is_group_member(atoi(s)))
	      {
		found = 1;
		break;
	      }
	  } 
	else if (strcmp(s, user) == 0) 
	  {
	    found = 1;
	    break;
	  }
      }
    
    if (!found) 
      {
        errormsg("%s: You are not authorized to make deltas.",
                 name.c_str());
        return false;
      }
  }
  return true;
}


/* Local variables: */
/* mode: c++ */
/* End: */