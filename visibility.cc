/*
 * visibility.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999, Free Software Foundation, Inc. 
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
 * Members of the class seq_state that require the use of a cssc_delta_table.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta-table.h"


void
seq_state::apply_include_list(const delta& d,
			      cssc_delta_table *pt,
			      bool bDebug)
{
  int len = d.included.length();
  for(int i = 0; i < len; i++)
    {
      const seq_no s = d.included[i];
      
      // A particular delta cannot have a LATER delta in 
      // its include list.
      ASSERT(s <= d.seq);
      
      if (!is_excluded(s))
	{
	  if (bDebug)
	    {
	      fprintf(stderr, "seq %lu includes seq %lu\n", d.seq, s);
	    }
	  set_included(s);
	  ASSERT(is_included(s));

	  // Recurse.
	  apply_include_list(pt->delta_at_seq(s), pt, bDebug);
	}
    } 		
}



void
seq_state::render_visibility(bool bDebug, cssc_delta_table * delta_table)
{
  seq_no seq = delta_table->highest_seqno();
  while (seq != 0)
    {
      bool bExcluded  = is_excluded(seq);
      bool bIncluded  = is_included(seq);
      bool bAncestral = is_ancestral(seq);

      bool bVisible =  (bIncluded || (bAncestral && !bExcluded));

      if (bDebug)
	{
	  if (bAncestral)
	    {
	      fprintf(stderr, "seq %lu: is ancestral\n", seq);
	    }
	  if (bIncluded)
	    {
	      fprintf(stderr, "seq %lu: is included\n", seq);
	    }
	  if (bExcluded)
	    {
	      fprintf(stderr, "seq %lu: is excluded\n", seq);
	    }
	  
	  if (bVisible)
	    {
	      fprintf(stderr, "seq %lu: is visible\n", seq);
	    }
	  else
	    {
	      fprintf(stderr, "seq %lu: is not visible\n", seq);
	    }
	}

      
      if (bVisible)
	{
	  set_visible(seq);
      
	  // OK, this delta is visible in the final result.  Apply its
	  // include and exclude list.  We are travelling from newest to
	  // oldest deltas.  Hence deltas which are ALREADY excluded or
	  // included are left alone.  Only deltas which have not yet been
	  // either included or excluded are messed with.
	  
	  const delta &d = delta_table->delta_at_seq(seq);


	  
	  apply_include_list(d, delta_table, bDebug);


	  
	  int len = d.excluded.length();
	  int i;
	  for(i = 0; i < len; i++)
	    {
	      const seq_no s = d.excluded[i];
	      if (s == seq)
		continue;
	      
	      // A particular delta cannot have a LATER delta in 
	      // its exclude list.
	      ASSERT(s <= seq);
	      
	      if (!is_included(s))
		{
		  if (bDebug)
		    {
		      fprintf(stderr, "seq %lu excludes seq %lu\n", seq, s);
		    }
		  set_excluded(s);
		  ASSERT(is_excluded(s));
		}
	    }
	}
      
      --seq;
    }
  
  if (bDebug)
    {
      fprintf(stderr, "seq_state::render_visibility() done.\n");
    }
}

