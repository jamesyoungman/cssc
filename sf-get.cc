/*
 * sf-get.cc: Part of GNU CSSC.
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
 * Members of class sccs_file used in getting deltas. 
 * 
 */

#ifdef __GNUC__
#pragma implementation "seqstate.h"
#endif

#include "cssc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"
#include "delta.h"
#include "delta-table.h"
#include "linebuf.h"
#include "bodyio.h"

// We use @LIBOBJS@ instead now...
// #ifndef HAVE_STRSTR
// #include "strstr.cc"
// #endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-get.cc,v 1.28 1999/05/16 16:53:17 james Exp $";
#endif




// sccs_file::finalise_seqstate
//
// This function makes the final desisions on the visibility of each
// delta.  Some of the work is delegated to
// seq_state::render_visibility().  The word "FINALISE" here is not
// indentded to imply that an object is deleted.

bool
sccs_file::finalise_seqstate(seq_state &state,
			    seq_no get_seq)
{
  bool bDebug = getenv("CSSC_SHOW_SEQSTATE") ? true : false;

  // We must include the version we are trying to get.
  // Mark it and its ancestors.
  seq_no seq;
  for (seq=get_seq; seq != 0; seq = delta_table->delta_at_seq(seq).prev_seq)
    {
      state.set_ancestral(seq);
    }

  state.render_visibility(bDebug, delta_table);
  
  if (bDebug)
    {
      fprintf(stderr,
	      "sccs_file::finalise_seqstate(seq_state &state, seq_no seq)"
	      " done\n");
    }
  return true;
}


bool
sccs_file::get(mystring gname, class seq_state &state,
	       struct subst_parms &parms,
#ifdef __GNUC__
	       subst_fn_t subst_fn,
#else
	       int (sccs_file::*subst_fn)(const char *,
					  struct subst_parms *,
					  const delta&) const,
#endif
	       int show_sid, int show_module, int debug,
	       bool no_decode /* = false */)
{
  ASSERT(mode != CREATE);
  ASSERT(mode != FIX_CHECKSUM);

  int (*outputfn)(FILE*,const cssc_linebuf*);
  if (flags.encoded && false == no_decode)
    outputfn = output_body_line_binary;
  else
    outputfn = output_body_line_text;
	

  if (!seek_to_body())
    return false;
  

  int first_command = read_line();
  if (first_command != 'I' && first_command != 'D')
    {
      corrupt("Expected '@I' or '@D'");
      return false;
    }
  check_arg();

  unsigned short first_delta = strict_atous( plinebuf->c_str() + 3);
  state.start(first_delta, first_command);

  FILE *out = parms.out;

  while (1) {
    int c = read_line();

    if (c == -1) { 
      /* EOF */
      break;
    }

    if (c == 0) {
      /* A non-control line */

      if (debug) {
	if (state.include_line()) {
	  putc('I', f);
	} else {
	  putc('D', f);
	}
	putc(' ', f);
      } else if (!state.include_line()) {
	continue;
      }

      parms.out_lineno++;

      if (show_module)
	fprintf(out, "%s\t", get_module_name().c_str());

      if (show_sid)
	{
	  seq_to_sid(state.active_seq()).print(out);
	  putc('\t', out);
	}

      int err;
      if (subst_fn != NULL)
	{
	  // If there is a cutoff date,
	  // prepare_seqstate() will take account of
	  // it.  We need the keyword substitution to
	  // take account of this and substitute the
	  // correct stuff.... so we figure out what
	  // delta has actually been selected here...


	  if (flags.encoded)
	    {
	      /*
	       * We ignore the possiblity of keyword substitution.
	       * I don't think "real" SCCS does keyword substitution
	       * for this case either -- James Youngman <jay@gnu.org>
	       */
	      err = outputfn(out, plinebuf);
	    }
	  else
	    {
	      err = (this->*subst_fn)(plinebuf->c_str(), &parms, parms.delta);
	      
	      if (fputc_failed(fputc('\n', out)))
		err = 1;
	    }
	}
      else
	{
	  if (!parms.found_id && plinebuf->check_id_keywords())
	    parms.found_id = 1;
	  err = outputfn(out, plinebuf);
	} 

      if (err)
	{
	  errormsg_with_errno("%s: Write error.", gname.c_str());
	  return false;
	}
      
      continue;
    }

    /* A control line */

    check_arg();
    seq_no seq = strict_atous(plinebuf->c_str() + 3);
    if (seq < 1 || seq > highest_delta_seqno()) {
      corrupt("Invalid serial number");
    }

    const char *msg = NULL;

    switch (c) {
    case 'E':
      msg = state.end(seq);
      break;

    case 'D':
    case 'I':
      msg = state.start(seq, c);
      break;
      
    default:
      corrupt("Unexpected control line");
      break;
    }

    if (msg != NULL) {
      corrupt(msg);
    }
  }

  fflush(out);
  return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
