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
static const char rcs_id[] = "CSSC $Id: sf-get.cc,v 1.32 2001/07/31 08:28:07 james_youngman Exp $";
#endif

bool
sccs_file::prepare_seqstate(seq_state &state, seq_no seq)
{
  bool bDebug = getenv("CSSC_SHOW_SEQSTATE") ? true : false;

  // We must include the version we are trying to get.
  state.set_included(seq);
  
  while (seq != 0)
    {
      int len;
      int i;

      bool bExcluded = state.is_excluded(seq);
      bool bIncluded = state.is_included(seq);

      bool bVisible = true;
      if (bExcluded)
        bVisible = false;
      else if (!bIncluded)
        bVisible = false;

      if (bDebug)
        {
          if (bExcluded)
            {
              fprintf(stderr, "seq %lu: is excluded\n", (unsigned long)seq);
            }
          if (bVisible)
            {
              fprintf(stderr, "seq %lu: is visible\n", (unsigned long)seq);
            }
          else
            {
              fprintf(stderr, "seq %lu: is not visible\n", (unsigned long)seq);
            }
        }

      
      if (bVisible)
        {
          // OK, this delta is visible in the final result.  Apply its
          // include and exclude list.  We are travelling from newest to
          // oldest deltas.  Hence deltas which are ALREADY excluded or
          // included are left alone.  Only deltas which have not yet been
          // either included or excluded are messed with.
          
          const delta &d = delta_table->delta_at_seq(seq);
          
          len = d.included.length();
          for(i = 0; i < len; i++)
            {
              const seq_no s = d.included[i];
              if (s == seq)
                continue;
              
              // A particular delta cannot have a LATER delta in 
              // its include list.
              ASSERT(s <= seq);
              
              if (!state.is_excluded(s))
                {
                  if (bDebug)
                    {
                      fprintf(stderr, "seq %lu includes seq %lu\n",
                              (unsigned long) seq,
                              (unsigned long) s);
                    }
                  state.set_included(s, true); 
                  ASSERT(state.is_included(s));
                }
            }           

          // If this seq was explicitly included, don't recurse for it 
          // (this fixes SourceForge bug number 111140).
          if (state.is_recursive(seq))
            {
              state.set_included(d.prev_seq);
            }
          
          len = d.excluded.length();
          for(i = 0; i < len; i++)
            {
              const seq_no s = d.excluded[i];
              if (s == seq)
                continue;
              
              // A particular delta cannot have a LATER delta in 
              // its exclude list.
              ASSERT(s <= seq);
              
              if (!state.is_included(s))
                {
                  if (bDebug)
                    {
                      fprintf(stderr, "seq %lu excludes seq %lu\n",
                              (unsigned long)seq,
                              (unsigned long)s);
                    }
                  state.set_excluded(s);
                  ASSERT(state.is_excluded(s));
                }
            }
        }
      
      --seq;
    }
  
  if (bDebug)
    {
      fprintf(stderr,
              "sccs_file::prepare_seqstate(seq_state &state, seq_no seq)"
              " done\n");
    }
  return true;
}


bool
sccs_file::get(mystring gname, class seq_state &state,
               struct subst_parms &parms,
               bool do_kw_subst,
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
  

  /* The following statement is not correct. */
  /* "@I 1" should start the body of the SCCS file */

  if (read_line() != 'I')
    {
      corrupt("Expected '@I'");
      return false;
    }
  check_arg();

  /* The check on the following line is certainly wrong, since
   * the first body line need not refer to the first delta.  For
   * example, SunOS 4.1.1's SCCS implementation doesn't always
   * start with ^AI 1.
   */
  unsigned short first_delta = strict_atous( plinebuf->c_str() + 3);
  state.start(first_delta, 'I'); /* 'I' means "insert". */

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
      if (do_kw_subst)
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
                // Mark Reynolds <mark@aoainc.com>: GCC 2.8.1 on VAX
                // Ultrix 4.2 doesn't seem to get this call right.
                // Since subst_fn is always write_subst anyway, we 
                // work around it by using the function pointer just as a 
                // boolean variable.   Yeuch.
                // 
                // 2001-07-30: get rid of all the cruft by using a boolean
                //             flag instead of a function pointer, for all
                //             systems. 
                err = write_subst(plinebuf->c_str(), &parms, parms.delta);
              
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
