/*
 * sf-get3.cc: Part of GNU CSSC.
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
 * Members of the class sccs_file used by get and delta.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta-iterator.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-get3.cc,v 1.8 1997/12/26 17:00:14 james Exp $";
#endif

/* Prepare a seqstate for use by marking which sequence numbers are
   to be included and which are to be excluded. */

void
sccs_file::prepare_seqstate(seq_state &state, sid_list include,
			    sid_list exclude, sccs_date cutoff_date)
{
  
  ASSERT(0 != delta_table);
  delta_iterator iter(delta_table);

  while (iter.next())
    {
      sid const &id = iter->id;
      
      if (include.member(id))
	{
	  state.include(iter->seq);
	}
      if (exclude.member(id))
	{
	  state.exclude(iter->seq);
	}
      if (cutoff_date.valid() && iter->date > cutoff_date)
	{
	  state.exclude(iter->seq);
	}
      
      ASSERT(0 != delta_table);
    }
  ASSERT(0 != delta_table);
}

/* Local variables: */
/* mode: c++ */
/* End: */
