/*
 * sf-get.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2002, 2003, 2004, 2007, 2008,
 *  2009, 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Members of class sccs_file used in getting deltas.
 *
 */

#include <config.h>

#include <string>
using std::string;

#include "cssc.h"

#include "body-scanner.h"
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

bool
sccs_file::prepare_seqstate_1(seq_state &state, seq_no seq)
{
  bool bDebug = getenv("CSSC_SHOW_SEQSTATE") ? true : false;

  /* new code */
#if 1
  seq_no y;

  // deltas descended from the version we want are wanted (unless excluded)
  y = seq;
  do
    {
      ASSERT(y <= seq);
      if (!delta_table->delta_at_seq_exists(y)) {
	  corrupt_file("missing sequence number %u", unsigned(y));
      }
      const delta &d = delta_table->delta_at_seq(y);
      if (d.prev_seq() == y) {
	  corrupt_file("sequence number %u cannot be its own predecessor", unsigned(y));
      } else if (d.prev_seq() > y) {
	  corrupt_file("sequene number %u has invalid (subsequent) predecessor %u",
		       unsigned(y), unsigned(d.prev_seq()));
      }
      state.set_included(d.prev_seq(), y, false);
      y = d.prev_seq();
    } while (y > 0);
  state.set_included(seq, (seq_no) seq_state::BY_DEFAULT, false);

  // Apply any inclusions
  for (y=seq; y>0; --y)
    {
      if (state.is_included(y))
	{
	  const delta &d = delta_table->delta_at_seq(y);

	  if (bDebug)
	    {
	      const std::vector<seq_no>::size_type len = d.get_included_seqnos().size();
	      fprintf(stderr,
		      "seq %d includes %lu other deltas...\n",
		      y, static_cast<unsigned long>(len));
	    }

	  for (auto s : d.get_included_seqnos())
	    {
	      if (s == y)
		continue;

	      // A particular delta cannot have a LATER delta in
	      // its include list.
	      ASSERT(s <= y);
	      state.set_included(s, y, true);
	      ASSERT(state.is_included(s));
	    }
	}
    }

  // Apply any exclusions
  for (y=1; y<=seq; ++y)
    {
      if (state.is_included(y))
      {
	const delta &d = delta_table->delta_at_seq(y);

	if (bDebug)
	  {
	    const std::vector<seq_no>::size_type len = d.get_excluded_seqnos().size();
	    fprintf(stderr,
		    "seq %d excludes %lu other deltas...\n",
		    y, static_cast<unsigned long>(len));
	  }

	for (auto s : d.get_excluded_seqnos())
	  {
	    if (s == y)
	      continue;

	    // A particular delta cannot have a LATER delta in
	    // its exclude list.
	    ASSERT(s <= y);
	    state.set_excluded(s, y);
	    ASSERT(state.is_excluded(s));
	  }
      }
    }


  // Apply any ignores
  // These are not recursive, so for example if version 1.6 ignored
  // version 1.2, the body lines for 1.1 will still be included.
  // (but what about any includes or excludes?)
  for (y=seq; y>0; --y)
    {
      if (state.is_included(y))
	{
	  const delta &d = delta_table->delta_at_seq(y);
	  if (bDebug)
	    {
	      const std::vector<seq_no>::size_type len = d.get_ignored_seqnos().size();
	      fprintf(stderr,
		      "seq %d ignores %lu other deltas...\n",
		      y, static_cast<unsigned long>(len));
	    }

	  for (auto s : d.get_ignored_seqnos())
	    {
	      if (s == y)
		continue;

	      ASSERT(s <= y);
	      state.set_ignored(s, y);

	      ASSERT(state.is_ignored(s));
	      ASSERT(!state.is_included(s));
	      ASSERT(!state.is_excluded(s));
	    }
	}
    }


  if (bDebug)
    {
      for (y=1; y<=seq; ++y)
	{
	  const char *msg;
	  if (state.is_ignored(y))
	    msg = "ignored";
	  else if (state.is_included(y))
	    msg = "included";
	  else
	    msg = "excluded";

	  fprintf(stderr, "seq_no %d: %s\n", y, msg);
	}
    }

#else

  // We must include the version we are trying to get.
  state.set_included(seq, (seq_no) seq_state::BY_DEFAULT, false);

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
	      if (bDebug)
		{
		  fprintf(stderr,
			  "seq %d includes %d other deltas...\n",
			  seq, len);
		}

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
                  state.set_included(s, seq, true);
                  ASSERT(state.is_included(s));
                }
            }

          len = d.excluded.length();
          for(i = 0; i < len; i++)
            {
	      if (bDebug)
		{
		  fprintf(stderr,
			  "se %d excludes %d other deltas...\n",
			  seq, len);
		}

              const seq_no s = d.excluded[i];
              if (s == seq)
                continue;

              // A particular delta cannot have a LATER delta in
              // its exclude list.
              ASSERT(s <= seq);

	      if (bDebug)
		{
		  fprintf(stderr, "seq %lu excludes seq %lu\n",
			  (unsigned long)seq,
			  (unsigned long)s);
		}
	      state.set_explicitly_excluded(s, seq);
	      ASSERT(state.is_excluded(s));
            }

          // If this seq was explicitly included, don't recurse for it
          // (this fixes SourceForge bug number 111140).
          if (state.is_recursive(seq))
            {
	      fprintf(stderr, "seq %lu is recursive; including seq %lu\n",
		      (unsigned long)seq,
		      (unsigned long)d.prev_seq);
	      state.set_included(d.prev_seq, seq, false);
	    }
	}

      --seq;
    }

#endif


  if (bDebug)
    {
      fprintf(stderr,
              "sccs_file::prepare_seqstate_1(seq_state &state, seq_no seq)"
              " done\n");
    }
  return true;
}

bool
sccs_file::get(const string& gname,
	       class seq_state &state,
               struct subst_parms &parms,
               bool do_kw_subst,
               int show_sid, int show_module, int debug,
               bool no_decode /* = false */,
	       bool for_edit /* = false */)
{
  ASSERT(mode != CREATE);
  ASSERT(mode != FIX_CHECKSUM);

  if (!edit_mode_ok(for_edit))	// "get -e" on BK files is not allowed
    return false;

  int (*outputfn)(FILE*,const cssc_linebuf*);
  if (flags.encoded && false == no_decode)
    outputfn = output_body_line_binary;
  else
    outputfn = output_body_line_text;

  auto subst = [this, &parms](const char *start, struct delta const& gotten_delta,
			bool force_expansion) -> int
    {
      return this->write_subst(start, &parms, gotten_delta, force_expansion);
    };

  return body_scanner_->get(gname, *delta_table, subst,
			    outputfn, flags.encoded, state, parms,
			    do_kw_subst, debug, show_module, show_sid);
}

/* Local variables: */
/* mode: c++ */
/* End: */
