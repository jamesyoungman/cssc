/*
 * sf-rmdel.c: Part of GNU CSSC.
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
 * Members of the class sccs_file used for marking a delta in the SCCS
 * files as removed.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-iterator.h"
#include "linebuf.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-rmdel.cc,v 1.7 1997/12/26 18:33:18 james Exp $";
#endif

static int
is_seqlist_member(seq_no seq, list<seq_no> const &seq_list) {
	int i;
	int len = seq_list.length();
	for(i = 0; i < len; i++) {
		if (seq == seq_list[i]) {
			return 1;
		}
	}
	return 0;
}


typedef enum { COPY, DELETE, INSERT} update_state;

static int
next_state(update_state& current, // this arg is MODIFIED!
	   int key)
{
  if (current == COPY)
    {
      switch (key)
	{
	case 'I':
	  current = INSERT;
	  return 1;
	case 'D':
	  current = DELETE;
	  return 1;
	}
    }
  else
    {
      if ('E' == key)
	{
	  current = COPY;
	  return 1;
	}
    }
  return 0;		// not expected.
}


void
sccs_file::rmdel(sid id)
{
  delta *d = find_delta(id);
  if (0 == d)
    {
      quit(-1, "%s: Specified SID not found in SCCS file.",
	   name.c_str());
    }
  seq_no seq = d->seq;

  delta_iterator iter(delta_table);
  while (iter.next())
    {
      if (iter->prev_seq == seq)
	{
	  quit(-1, "%s: Specified SID has a successor.",
	       name.c_str());
	}
      if (is_seqlist_member(seq, iter->included)
	  || is_seqlist_member(seq, iter->excluded)
	  || is_seqlist_member(seq, iter->ignored))
	{
	  quit(-1, "%s: Specified SID is used in another delta.",
	       name.c_str());
	}
    }

  d->type = 'R';
	
  FILE *out = start_update();
  if (write(out))
    {
      xfile_error("Write error.");
    }

  update_state state = COPY;
  int c;
  while ( (c=read_line()) != -1)
    {
      if (0 != c)
	{
	  check_arg();
	  if (strict_atous(plinebuf->c_str() + 3) == seq)
	    {
	      if (!next_state(state, c))
		corrupt("Unexpected control line");
	    }
	  else if (state == INSERT)
	    {
	      corrupt("Non-terminal delta!?!");
	    }
	}
      else if (state != INSERT)
	{
	  fputs(plinebuf->c_str(), out);
	  putc('\n', out);
	}
    }
  // We should end the file after an 'E', that is,
  // in the 'COPY' state.
  if (state != COPY)
    {
      corrupt("Unexpected EOF");
    }
  
  end_update(out);
}		      
	
/* Local variables: */
/* mode: c++ */
/* End: */
