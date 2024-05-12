/*
 * body-scanner.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 2019, 2024 Free Software Foundation, Inc.
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
 */
#include "config.h"
#include "cssc.h"

#include <string.h>
#include <cstdio>
#include <memory>
#include <system_error>

#include "body-scanner.h"
#include "delta.h"
#include "delta-table.h"
#include "diff-state.h"
#include "failure.h"
#include "failure_macros.h"
#include "failure_or.h"
#include "filediff.h"
#include "filepos.h"
#include "ioerr.h"
#include "linebuf.h"
#include "seqstate.h"
#include "subst-parms.h"
#include "quit.h"


using cssc::Failure;
using cssc::FailureOr;
using cssc::make_failure_builder_from_errno;

sccs_file_body_scanner::sccs_file_body_scanner(const std::string& filename,
					       FILE*f, off_t body_pos, long line_number)
  : sccs_file_reader_base(filename, f, sccs_file_location(filename, line_number)),
    f_(f),
    body_start_(body_pos),
    start_(filename, line_number)
{
}

sccs_file_body_scanner::~sccs_file_body_scanner()
{
  // Access to f_ is read-only, so if the close fails there cannot be
  // any data loss.  Hence ignoring a failure to close the file isn't
  // going to astonish the user.
  (void)fclose(f_);
  f_ = nullptr;
}

cssc::Failure sccs_file_body_scanner::seek_to_body()
{
  if (fseek(f_, body_start_, SEEK_SET) != 0)
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "fseek failed on " << name();
    }
  here_.set_line_number(start_.line_number());
  return cssc::Failure::Ok();
}


cssc::Failure
sccs_file_body_scanner::get(const std::string& gname,
			    const cssc_delta_table& delta_table,
			    std::function<cssc::Failure(const char *start,
							struct delta const& gotten_delta,
							bool force_expansion)> write_subst,
			    cssc::Failure (*outputfn)(FILE*,const cssc_linebuf*),
			    bool encoded,
			    class seq_state &state,
			    struct subst_parms &parms,
			    bool do_kw_subst, bool /*debug*/, bool show_module, bool show_sid)
{
  const seq_no highest_delta_seqno = delta_table.highest_seqno();

  cssc::Failure seek = seek_to_body();
  if (!seek.ok())
    return seek;

  /* The following statement is not correct. */
  /* "@I 1" should start the body of the SCCS file */

  char line_type;
  cssc::FailureOr<char> fol = read_line();
  if (!fol.ok())
    {
      if (isEOF(fol.fail()))
	corrupt(here(), "Expected '@I'");
      else
	return fol.fail();
    }
  line_type = *fol;
  if (line_type != 'I')
    {
      corrupt(here(), "Expected '@I'");
      /*NOTREACHED*/
    }
  check_arg();

  /* The check on the following line is certainly wrong, since
   * the first body line need not refer to the first delta.  For
   * example, SunOS 4.1.1's SCCS implementation doesn't always
   * start with ^AI 1.
   */
  unsigned short first_delta = strict_atous(here(), plinebuf->c_str() + 3);
  state.start(first_delta, 'I'); /* 'I' means "insert". */

  FILE *out = parms.out;

  while (1) {
    fol = read_line();
    if (!fol.ok())
      {
	if (isEOF(fol.fail()))
	  break;
	corrupt(here(), "Unexpected end-of-file");
      }
    line_type = *fol;
    if (line_type == 0) {
      /* A non-control line */

      if (!state.include_line())
	{
	  continue;
	}

      parms.out_lineno++;

      if (show_module)
        fprintf(out, "%s\t", parms.get_module_name().c_str());

      if (show_sid)
        {
	  const seq_no active = state.active_seq();
	  const struct delta& d = delta_table.delta_at_seq(active);
	  d.id().print(out);
          putc('\t', out);
        }
      if (do_kw_subst && !encoded)
	{
	  cssc::Failure wrote = write_subst(plinebuf->c_str(), parms.delta, false);
	  if (!wrote.ok())
	    {
	      wrote = cssc::make_failure_builder(wrote)
		<< "failed to write to " << gname;
	    }
	  if (fputc_failed(fputc('\n', out)))
	    {
	      wrote = Update(wrote, cssc::make_failure_builder_from_errno(errno)
			     << "failed to write to " << gname);
	    }
	  if (!wrote.ok())
	    return wrote;
	}
      else
	{
	  if (!do_kw_subst)
	    {
	      if (!parms.found_id && plinebuf->check_id_keywords())
		  parms.found_id = 1;
	    }
	  cssc::Failure wrote = outputfn(out, plinebuf.get());
	  if (!wrote.ok())
	    {
	      return cssc::make_failure_builder(wrote)
		<< "failed to write to " << gname;
	    }
	}
      continue;
    }

    /* A control line */

    check_arg();
    seq_no seq = strict_atous(here(), plinebuf->c_str() + 3);
    if (seq < 1 || seq > highest_delta_seqno) {
      corrupt(here(), "Invalid serial number %u converted from '%s'", unsigned(seq), plinebuf->c_str());
      /*NOTREACHED*/
    }

    auto badstate = [this](const std::string& msg)
      {
	corrupt(here(), "%s", msg.c_str());
	/*NOTREACHED*/
      };

    switch (line_type) {
    case 'E':
      {
	auto outcome = state.end(seq);
	if (!outcome.first)
	  {
	    badstate(outcome.second);
	    /*NOTREACHED*/
	  }
      }
      break;

    case 'D':
    case 'I':
      {
	auto outcome = state.start(seq, line_type);
	if (!outcome.first)
	  {
	    badstate(outcome.second);
	    /*NOTREACHED*/
	  }
      }
      break;

    default:
      corrupt(here(), "Unexpected control line");
      /*NOTREACHED*/
      break;
    }
  }

  if (fflush_failed(fflush(out)))
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "failed to flush output to " << gname;
    }
  return cssc::Failure::Ok();	// success
}

namespace
{
  template <class T, class U> U convert(T val)
  {
    return static_cast<U>(val);
  }

  template <class T> T convert(T val)
  {
    return val;
  }

  void line_too_long(long int max_len, size_t actual_len)
  {
    errormsg("Input line is %lu characters long but "
	     "the maximum allowed SCCS file line length "
	     "is %ld characters (see output of delta -V)\n",
	     // TODO: switch to ostream based formatting
	     // to void the need for this kind of cast
	     // (especially since size_t and unsigned long)
	     // may be of different width.
	     convert<unsigned long>(actual_len),
	     max_len);
  }

}  // namespace

delta_result
sccs_file_body_scanner::delta(const std::string& dname,
			      const std::string& file_to_diff,
			      seq_no highest_delta_seqno,
			      seq_no new_seq,
			      seq_state *sstate, FILE *out,
			      bool display_diff_output)
{
  delta_result result;
  if (!seek_to_body().ok())  // prepare to read the body for predecessor.
    {
      result.success = false;
      return result;
    }

  FileDiff differ(dname.c_str(), file_to_diff.c_str());
  FILE *diff_out = differ.start();
  class diff_state dstate(diff_out, display_diff_output);

  result.success = [this, &result, highest_delta_seqno, new_seq, sstate, &dstate, out]() -> bool
    {
      const unsigned long int len_max = max_sfile_line_len();
      // We have to continue while there is data on the input s-file,
      // or data from the diff, so we don't just stop when read_line()
      // returns -1.
      while (1)
	{
	  char c;
	  // read line from the old body.
	  FailureOr<char> done = read_line();
	  bool got_line;
	  if (!done.ok())
	    {
	      if (!isEOF(done.fail()))
		{
		  // TODO: better error diagnosis.
		  return false;
		}
	      got_line = false;
	    }
	  else
	    {
	      c = *done;
	      got_line = true;
	    }

#ifdef JAY_DEBUG
	  fprintf(stderr, "input: %s\n", plinebuf->c_str());
#endif
	  if (got_line && c != 0)
	    {
	      // it's a control line.
	      seq_no seq = strict_atous(here(), plinebuf->c_str() + 3);

#ifdef JAY_DEBUG
	      fprintf(stderr, "control line: %c %lu\n", c, (unsigned)seq);
#endif

	      if (seq < 1 || seq > highest_delta_seqno)
		{
		  corrupt(here(), "Invalid sequence number %u", unsigned(seq));
		}

	      auto badstate = [this](const std::string& msg)
		{
		  corrupt(here(), "%s", msg.c_str());
		};
	      switch (c)
		{
		case 'E':
		  {
		    auto outcome = sstate->end(seq);
		    if (!outcome.first)
		      {
			badstate(outcome.second);
		      }
		  }
		  break;

		case 'D':
		case 'I':
		  {
		    auto outcome = sstate->start(seq, c);
		    if (!outcome.first)
		      {
			badstate(outcome.second);
		      }
		  }
		  break;

		default:
		  corrupt(here(), "Unexpected control line '%c'", c);
		  break;
		}
	    }
	  else if (sstate->include_line())
	    {
#ifdef JAY_DEBUG
	      fprintf(stderr, "body line, inserting\n");
#endif


	      // We just read a body line and prev delta is in insert
	      // mode.  We need to decide if this line must also go into
	      // this version.  If not, we need to emit delete commands.
	      // On the other hand, we may need to insert data before it.
	      // But if we just want to insert it into this version too,
	      // we still need to count it as an unchanged line.

	      diffstate action;

	      do
		{
		  // decide what to do with this line.
		  // process() also emits the neccesary command
		  // (insert, delete, end).
		  action = dstate.process(out, new_seq);
		  switch (action)
		    {

		    case diffstate::DELETE:
		      // signal that we want to delete that line,
		      // and break out of this inner loop (we do
		      // that by leaving the INSERT state).  The
		      // outer loop will deal with copying the line
		      // into the output, after we've emitted our
		      // delete marker; dstate.process() already
		      // did that.  We still need to copy the line
		      // into the output because even though the line
		      // is deleted in this delta, it still needs to
		      // be there for previous deltas.  That's what
		      // history files are for.

		      // Sanity check: if we're deleting a line, there
		      // must have been one on the input.
		      ASSERT(c != -1);
#ifdef JAY_DEBUG
		      fprintf(stderr, "diff_state::DELETE\n");
#endif
#ifdef DEBUG_FILE
		      fprintf(df, "%4d %4d - %s\n",
			      dstate.in_line(),
			      dstate.out_line(),
			      linebuf.c_str());
		      fflush(df);
#endif
		      ++result.deleted;
		      break;

		    case diffstate::INSERT:
		      {
			// We're inserting some data.   Emit that
			// data.  When we've done that, we'll either
			// go to the DELETE state (i.e. changed text)
			// or the NOCHANGE state (simple insertion).
			// Until then, copy data from the diff output
			// into our own output.
#ifdef JAY_DEBUG
			fprintf(stderr, "diff_state::INSERT\n");
#endif
#ifdef DEBUG_FILE
			fprintf(df, "%4d %4d + %s",
				dstate.in_line(),
				dstate.out_line(),
				dstate.get_insert_line());
			fflush(df);
#endif
			++result.inserted;

			auto pline = dstate.get_insert_line();
			auto len = strlen(pline);
			if (len)
			  len -= 1u;  // newline char should not contribute.

			if (0 == len_max || len < len_max)
			  {
			    if (fputs_failed(fputs(pline, out)))
			      {
				return false;
			      }
			  }
			else
			  {
			    // The line is too long.
			    line_too_long(len_max, len);
			    return false;
			  }
			break;
		      }

		    case diffstate::END:
#ifdef JAY_DEBUG
		      fprintf(stderr, "diff_state::END\n");
#endif
		      if (c == -1)
			{
			  break;
			}
		      /* FALLTHROUGH */
		    case diffstate::NOCHANGE:
		      // line unchanged - so there must have been an input line,
		      // so we cannot be at the end of the data.
		      ASSERT(c != -1);
#ifdef DEBUG_FILE
		      fprintf(df, "%4d %4d   %s\n",
			      dstate.in_line(),
			      dstate.out_line(),
			      linebuf.c_str());
		      fflush(df);
#endif
		      ++result.unchanged;
		      break;

		    default:
		      abort();
		    }
		} while (action == diffstate::INSERT);

#ifdef JAY_DEBUG
	      fprintf(stderr, "while (action==diff_state::INSERT) loop ended.\n");
#endif
	    }

	  if (!got_line)
	    {
	      // If we've exhausted the input we may still have a block to
	      // insert at the end.
	      while (diffstate::INSERT == dstate.process(out, new_seq))
		{
		  ++result.inserted;

		  auto pline = dstate.get_insert_line();
		  auto len = strlen(pline);
		  if (len)
		    len -= 1u;      // newline char should not contribute.

		  if (0 == len_max
		      || len < len_max
		      )
		    {
		      if (fputs_failed(fputs(pline, out)))
			{
			  return false;
			}
		    }
		  else
		    {
		      // The line is too long.
		      line_too_long(len_max, len);
		      return false;
		    }
		}

	      break;
	    }


#ifdef JAY_DEBUG
	  fprintf(stderr, "-> %s\n", plinebuf->c_str());
#endif
	  fputs(plinebuf->c_str(), out);
	  putc('\n', out);
	}
      return true;
    }();

  differ.finish(diff_out); // "give back" the FILE pointer.
  ASSERT(nullptr == diff_out);
  return result;
}

/* Implements prs :GB:*/
Failure
sccs_file_body_scanner::emit_raw_body(FILE* out, const char *outname)
{
  auto emitline = [out](const char* s) -> Failure
    {
      TRY_PUTS(fputs(s, out));
      TRY_PUTC(putc('\n', out));
      return Failure::Ok();
    };

  TRY_OPERATION(seek_to_body()); // error message already emitted.
  for(;;)
    {
      FailureOr<char> got = read_line();
      if (!got.ok())
	{
	  if (isEOF(got.fail()))
	    return Failure::Ok();
	  else
	    return got.fail();
	}
      Failure f = emitline(plinebuf->c_str());
      if (!f.ok())
	{
	  return cssc::make_failure_builder(f)
	    << "failed to write to " << outname;
	}
    }
  return Failure::Ok();
}

cssc::Failure
sccs_file_body_scanner::print_body(FILE *out, const std::string& outname)
{
  bool ret = true;

  // When pos_saver goes out of scope the file position on "f_" is restored.
  FilePosSaver pos_saver(f_);
  Failure done =  seek_to_body();
  if (!done.ok())
    {
      return cssc::make_failure_builder(done).diagnose();
    }


  auto write_err = [outname](int saved_errno) -> Failure
    {
      return make_failure_builder_from_errno(saved_errno)
      .diagnose() << "write failed on " << outname;
    };
  auto read_err = [this](int saved_errno) -> Failure
    {
      return make_failure_builder_from_errno(saved_errno)
      .diagnose() << "read failed on " << name().c_str();
    };

  if (putc_failed(putc('\n', out)))
    return write_err(errno);

  int ch;
  while ( ret && (ch=getc(f_)) != EOF )
    {
      if ('\001' == ch)
	{
	  if (fputs_failed(fputs("*** ", out)))
	    {
	      return write_err(errno);
	    }
	}
      else if ('\n' == ch)
	{
	  int peek = getc(f_);

	  if ('\001' == peek)
	    {
	      if (EOF == ungetc(peek, f_))
		{
		  return cssc::make_failure_builder_from_errno(errno)
		    .diagnose() << "ungetc failed on " << name().c_str();
		}
	      if (putc_failed(putc('\n', out)))
		return write_err(errno);
	    }
	  else if (EOF == peek)
	    {
	      if (putc_failed(putc('\n', out)))
		return write_err(errno);
	      break;
	    }
	  else
	    {
	      if (EOF == ungetc(peek, f_))
		{
		  return cssc::make_failure_builder_from_errno(errno)
		    .diagnose() << "ungetc failed on " << name().c_str();
		}
	      if (fputs_failed(fputs("\n\t", out)))
		{
		  return write_err(errno);
		}
	    }
	}
      else
	{
	  if (putc_failed(putc(ch, out)))
	    {
	      return write_err(errno);
	    }
	}
    }
  if (ferror(f_))		// read error is fatal.
    {
      return read_err(errno);
    }
  // When pos_saver goes out of scope the file position is restored.
  return Failure::Ok();
}


namespace
{
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
}  // namespace


cssc::Failure
sccs_file_body_scanner::remove(FILE *out, seq_no seq)
{
  update_state state = COPY;
  for (;;)
    {
      FailureOr<char> got = read_line();
      if (!got.ok())
	{
	  if (isEOF(got.fail()))
	    break;
	  else
	    return got.fail();
	}

      const char c = *got;
      if (0 != c)
	{
	  check_arg();
	  if (strict_atous(here(), plinebuf->c_str() + 3) == seq)
	    {
	      if (!next_state(state, c))
		{
		  return cssc::make_failure_builder(cssc::errorcode::HistoryFileCorrupt)
		    << "unexpected control line at " << here();
		}
	    }
	  else if (state == INSERT)
	    {
	      return cssc::make_failure_builder(cssc::errorcode::HistoryFileCorrupt)
		<< "non-terminal delta at " << here();
	    }
	  else
	    {
	      if (fputs_failed(fputs(plinebuf->c_str(), out))
		  || putc_failed(putc('\n', out)))
		{
		  return cssc::make_failure_builder_from_errno(errno)
		    << "write error on output file";
		}
	    }
	}
      else if (state != INSERT)
	{
	  if (fputs_failed(fputs(plinebuf->c_str(), out))
	      || putc_failed(putc('\n', out)))
	    {
	      return cssc::make_failure_builder_from_errno(errno)
		    << "write error on output file";
	    }
	}
    }

  // We should end the file after an 'E', that is,
  // in the 'COPY' state.
  if (state != COPY)
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "unexpected EOF at " << here();
    }
  return cssc::Failure::Ok();
}

std::unique_ptr<sccs_file_body_scanner>
make_unique_sccs_file_body_scanner(const std::string& filename,
				   FILE*f,
				   off_t body_pos,
				   long body_pos_line_number)
{
#if __cplusplus >= 201402L
  return std::make_unique<sccs_file_body_scanner>(filename, f, body_pos, body_pos_line_number);
#else
  return std::unique_ptr<sccs_file_body_scanner>(new sccs_file_body_scanner(filename, f, body_pos, body_pos_line_number));
#endif
}
