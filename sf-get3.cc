/*
 * sf-get3.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999 Free Software Foundation, Inc. 
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
 * Members of the class sccs_file used by get and delta.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta-iterator.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-get3.cc,v 1.12 1999/04/21 22:19:12 james Exp $";
#endif

/* Prepare a seqstate for use by marking which sequence numbers are to
 * be included and which are to be excluded.
 */

bool
sccs_file::prepare_seqstate(seq_state &state, sid_list include,
			    sid_list exclude, sccs_date cutoff_date)
{
  
  ASSERT(0 != delta_table);
  const_delta_iterator iter(delta_table);

  while (iter.next())
    {
      sid const &id = iter->id;

      if (include.member(id))	// explicitly included on command line
	{
	  state.set_explicitly_included(iter->seq);
	}
      else if (exclude.member(id)) // explicitly included on command line
	{
	  state.set_explicitly_excluded(iter->seq);
	}
      else if (cutoff_date.valid() && iter->date > cutoff_date)
	{
	  // Delta not explicitly included/excluded by the user, but
	  // if it is newer than the cutoff date, we don't want it.
	  // This is the feature that allows us to retrieve a delta
	  // that was current at some time in the past.
	  state.set_excluded(iter->seq);
	}
      
      ASSERT(0 != delta_table);
    }
  ASSERT(0 != delta_table);
  
  return true;
}


/* Local variables: */
/* mode: c++ */
/* End: */
