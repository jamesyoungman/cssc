/*
 * sf-prt.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2004, 2007, 2008, 2009, 2010,
 *  2011, 2014, 2019 Free Software Foundation, Inc.
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
 * Members of the class sccs_file for doing sccs-prt.
 */


#include <config.h>

#include <errno.h>

#include "cssc.h"
#include "failure.h"
#include "ioerr.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "filepos.h"
#include "delta.h"
#include "delta-iterator.h"


using cssc::make_failure_from_errno;
using cssc::Failure;

#include <unistd.h>		// SEEK_SET on SunOS.

#define TRY_PRINTF(printf_expression) \
  do { int result = (printf_expression); \
  if (fprintf_failed(result)) { \
    return cssc::make_failure_from_errno(errno); \
  }						 \
  } while (0)

#define TRY_PUTS(puts_expression) \
  do { int result = (puts_expression); \
  if (fputs_failed(result)) { \
    return cssc::make_failure_from_errno(errno); \
  }						 \
  } while (0)

#define TRY_PUTC(putc_expression) \
  do { int result = (putc_expression); \
  if (fputc_failed(result)) { \
    return cssc::make_failure_from_errno(errno); \
  }						 \
  } while (0)

#define TRY_OPERATION(expression)		\
  do { Failure done = (expression);		\
    if (!done.ok()) {				\
    return done;				\
  }						\
  } while (0)

template<class Container>
static cssc::Failure
print_string_list(FILE *out,
		  const Container& l,
		  const char* pre,
		  const char* post,
		  const char* dflt)
{
  const std::vector<std::string>::size_type len = l.size();

  if (0 == len)
    {
      TRY_PRINTF(fprintf(out, "%s%s", pre, dflt));
    }
  else
    {
      for (std::vector<std::string>::size_type i = 0; i < len; i++)
	{
	  TRY_PRINTF(fprintf(out, "%s%s", pre, l[i].c_str()));
	  if (i < len-1)
	    {
	      TRY_PRINTF(fprintf(out, "%s", post));
	    }
	}
    }
  return cssc::Failure::Ok();
}

cssc::Failure print_flag(FILE *out, const char *fmt, std::string flag, int& count)
{
  if (!flag.empty())
    {
      ++count;
      TRY_PRINTF(fprintf(out, fmt, flag.c_str()));
    }
  return cssc::Failure::Ok();
}

cssc::Failure print_flag(FILE *out, const char *fmt, const std::string* pflag, int& count)
{
  // We consider a flag which is set to an empty string still to be set.
  // An example is the v flag; lines of the form "^Af v" should still set
  // the v flag to an empty string.
  if (pflag)
    {
      ++count;
      TRY_PRINTF(fprintf(out, fmt, pflag->c_str()));
    }
  return cssc::Failure::Ok();
}

cssc::Failure print_flag(FILE *out, const char *fmt,  int flag, int& count)
{
  if (flag)
    {
      ++count;
      TRY_PRINTF(fprintf(out, fmt, flag ? "yes" : "no"));
    }
  return cssc::Failure::Ok();
}

Failure print_flag(FILE *out, const char *fmt,  sid flag, int& count)
{
  if (flag.valid())
    {
      ++count;
      TRY_PRINTF(fprintf(out, "%s", fmt));
      Failure printed = flag.print(out);
      if (!printed.ok())
	return printed;
      TRY_PUTC(putc('\n', out));
    }
  return cssc::Failure::Ok();
}

cssc::Failure print_flag(FILE *out, const char *fmt,  release flag, int& count)
{
  if (flag.valid())
    {
      ++count;
      TRY_PRINTF(fprintf(out, "%s", fmt));
      Failure printed = flag.print(out);
      if (!printed.ok())
	return printed;
      TRY_PRINTF(fprintf(out, "\n"));
    }
  return cssc::Failure::Ok();
}

bool sccs_file::cutoff::excludes_delta(sid /* s */,
				       sccs_date date,
				       bool& stop_now) const
{
  stop_now = false;

  if (!enabled)
    return false;

  if (first_accepted.valid())
    {
      // stop_now = stop_now || (date <= first_accepted);
      if (date < first_accepted)
	return true;
    }
  if (last_accepted.valid())
    {
      // Don't set stop_now, since we have not even got to the first
      // one that's not excluded, yet.
      if (date > last_accepted)
	return true;
    }
  if (cutoff_delta)
    {
      stop_now = stop_now || (date <= cutoff_delta->date());
      if (date < cutoff_delta->date())
	return true;
    }
  return false;
}

cssc::Failure
sccs_file::cutoff::print(FILE *out) const
{
  TRY_PRINTF(fprintf(out, "cutoff: "));
  if (enabled)
    {
      TRY_PUTS(fputs("enabled ", out));
      if (most_recent_sid_only)
	{
	  TRY_PUTS(fputs("most-recent-only ", out));
	}
      TRY_PUTS(fputs("cutoff_sid='", out));
      if (cutoff_sid.valid())
	{
	  Failure printed = cutoff_sid.print(out);
	  if (!printed.ok())
	    return printed;
	}
      else
	{
	  TRY_PUTS(fputs("(invalid)", out));
	}

      TRY_PUTS(fputs("' first_accepted='", out));
      if (first_accepted.valid())
	{
	  Failure printed = first_accepted.printf(out, 'D');
	  if (printed.ok())
	    return printed;
	  TRY_PRINTF(fprintf(out, " "));
	  printed = first_accepted.printf(out, 'T');
	  if (printed.ok())
	    return printed;
	}
      else
	{
	  TRY_PUTS(fputs("(invalid)'", out));
	}
      TRY_PUTS(fputs("' last_accepted='", out));
      if (last_accepted.valid())
	{
	  Failure printed = last_accepted.printf(out, 'D');
	  if (printed.ok())
	    return printed;
	  TRY_PRINTF(fprintf(out, " "));
	  printed = last_accepted.printf(out, 'T');
	  if (printed.ok())
	    return printed;
	}
      else
	{
	  TRY_PUTS(fputs("(invalid)'", out));
	}

      if (cutoff_delta)
	{
	  TRY_PUTS(fputs("' cutoff_delta->date='", out));
	  if (cutoff_delta->date().valid())
	    {
	      Failure printed = last_accepted.printf(out, 'D');
	      if (!printed.ok())
		return printed;
	      TRY_PRINTF(fprintf(out, " "));
	      printed = last_accepted.printf(out, 'T');
	      if (!printed.ok())
		return printed;
	    }
	  else
	    {
	      TRY_PUTS(fputs("(invalid)'", out));
	    }
	}
      TRY_PUTS(fputs("'\n", out));
    }
  else
    {
      TRY_PUTS(fputs("disabled\n", out));
    }
  return cssc::Failure::Ok();
}

sccs_file::cutoff::cutoff()
  : enabled(false), most_recent_sid_only(false),
    cutoff_sid(sid()),
    cutoff_delta(NULL)
{
  // all done above.
}


static Failure
print_seq_list(FILE *out, std::vector<seq_no> const &list)
{
  bool first = true;
  for (const auto& seq : list)
    {
      TRY_PRINTF(fprintf(out, "%s%u", (first ? "" : " "), seq));
      first = false;
    }
  return Failure::Ok();
}


Failure
sccs_file::prt(FILE *out,
	       cutoff exclude,	      // -y, -c, -r
	       delta_selector selector,	// -a => delta_selector::all
	       int print_body,	      // -b
	       int print_delta_table, // -d
	       int print_flags,	      // -f
	       int incl_excl_ignore,  // -i
	       int first_line_only,   // -s
	       int print_desc,	      // -t
	       int print_users) const // -u

{
  const int suppress_newlines = exclude.enabled;
  const char* nl_sep = suppress_newlines ? " " : "\n";

  if (print_delta_table)
    {
      if (exclude.enabled)
	{
	  if (exclude.most_recent_sid_only)
	    {
	      find_most_recent_sid(exclude.cutoff_sid,
				   exclude.first_accepted);
	    }
	  if (exclude.cutoff_sid.valid())
	    exclude.cutoff_delta = find_delta(exclude.cutoff_sid);
	}

      bool stop_now = false;
      const_delta_iterator iter(delta_table.get(), selector);

      while (!stop_now && iter.next())
	{
	  if (exclude.excludes_delta(iter->id(), iter->date(), stop_now))
	    continue;

	  // Unless -a was specified, don't print removed deltas.  But
	  // they also should not be returned by the iterator.
	  if (iter->removed())
	    {
	      ASSERT(selector == delta_selector::all);
	    }

	  if (exclude.enabled)	// -y, -c, or -r option.
	    {
	      TRY_PRINTF(fprintf(out, "%s:\t", name.c_str()));
	    }
	  else
	    {
	      TRY_PUTC(putc('\n', out));
	    }

	  // Print the stuff from the delta...
	  TRY_PRINTF(fprintf(out, "%c ", iter->get_type()));
	  Failure printed = iter->id().print(out);
	  if (!printed.ok())
	    return printed;
	  TRY_PUTC(putc('\t', out));
	  printed = iter->date().printf(out, 'D');
	  if (!printed.ok())
	    return printed;
	  TRY_PUTC(putc(' ', out));
	  printed = iter->date().printf(out, 'T');
	  if (!printed.ok())
	    return printed;
	  TRY_PRINTF(fprintf(out, " %s\t%hu %hu",
			     iter->user().c_str(),
			     (unsigned short)iter->seq(),
			     (unsigned short)iter->prev_seq()));
	  TRY_PRINTF(fprintf(out, "\t%05lu/%05lu/%05lu",
			     iter->inserted(), iter->deleted(), iter->unchanged()));

	  if (!first_line_only)
	    {
	      if (incl_excl_ignore)
		{
		  // TODO: find an elegant solution to this problem.
		  //
		  // REAL SCCS prints an "Included" line in the output
		  // if it sees "^Ai " and "an "Excluded" line if it
		  // sees "^Ax ", even if the rest of the lines was
		  // blank.  For CSSC, the rest of these lines is
		  // built into a list of "seq_no"s. and so if the
		  // line is blank the data structure is empty and
		  // nothing is printed.  This is a behavioural
		  // difference that to fix seems to require a real
		  // kludge.
		  if (!iter->get_included_seqnos().empty())
		    {
		      TRY_PUTS(fputs(nl_sep, out)); // either newline or space.
		      TRY_PRINTF(fprintf(out, "Included:\t"));
		      Failure printed = print_seq_list(out, iter->get_included_seqnos());
		      if (!printed.ok())
			return printed;
		    }
		  else if (iter->has_includes())
		    {
		      TRY_PUTS(fputs(nl_sep, out)); // either newline or space.
		      TRY_PRINTF(fprintf(out, "Included:\t"));
		    }
		  if (!iter->get_excluded_seqnos().empty())
		    {
		      TRY_PUTS(fputs(nl_sep, out)); // either newline or space.
		      TRY_PRINTF(fprintf(out, "Excluded:\t"));
		      Failure printed = print_seq_list(out, iter->get_excluded_seqnos());
		      if (!printed.ok())
			return printed;
		    }
		  else if (iter->has_excludes())
		    {
		      TRY_PUTS(fputs(nl_sep, out)); // either newline or space.
		      TRY_PRINTF(fprintf(out, "Excluded:\t"));
		    }
		}
	      // Print any MRs and then the comments.
	      if (!iter->mrs().empty())
		{
		  TRY_PUTS(fputs(nl_sep, out));	// either newline or space.
		  Failure printed = print_string_list(out, iter->mrs(), "MRs:\t", nl_sep, "");
		  if (!printed.ok())
		    return printed;
		}
	      if (!iter->comments().empty())
		{
		  TRY_PUTS(fputs(nl_sep, out));	// either newline or space.
		  Failure printed = print_string_list(out, iter->comments(), "", nl_sep, "");
		  if (!printed.ok())
		    return printed;
		}
	    }
	  TRY_PUTC(putc('\n', out));
	}
    }

  // global stuff.
  if (print_users)
    {
      TRY_PRINTF(fprintf(out, "\n Users allowed to make deltas -- \n"));
      Failure printed = print_string_list(out, users, "\t", "\n", "everyone");
      if (!printed.ok())
	return printed;
      TRY_PUTC(putc('\n', out));
    }

  if (print_flags)
    {
      // PROBLEM:
      //
      // Those SCCS flags that have no "value" field, that is, those
      // which are either on or off, have a tab printed immediately
      // before their newlines...
      //
      // Except the "encoded" flag, which doesn't have a newline
      // printed after it, for some reason.
      //
      // But we emulate even that bug :-)
      //
      int flag_count = 0;
      TRY_PRINTF(fprintf(out, "\nFlags --\n"));
      if (flags.branch)
	{
	  // "Real" SCCS prints a TAB after "branch", so we do too.
	  TRY_PRINTF(fprintf(out, "\tbranch\t\n"));
	  ++flag_count;
	}

      Failure printed = print_flag(out, "\tceiling\t", flags.ceiling, flag_count);
      if (!printed.ok())
	return printed;
      printed = print_flag(out, "\tdefault SID\t", flags.default_sid, flag_count);
      if (!printed.ok())
	return printed;

      // No newline after this one;  odd, but it's what "real"
      // SCCS does.
      if (flags.encoded)
	{
	  TRY_PRINTF(fprintf(out, "\tencoded"));
	  ++flag_count;
	}

#if 0
      // The 'x' flag is a SCO OpenServer extension.
      // SCO OpenServer 5.0.6 has no "prt" command, but
      // SCO Unixware 8.0.0 does have it.  However, it
      // does not print anything if the x flag is set.
      // Hence for compatibility, we don't do that either.
      TRY_OPERATION(print_flag(out, "\texecutable\t\n",
			       flags.executable, flag_count));
#endif

      TRY_OPERATION(print_flag(out, "\tfloor\t", flags.floor, flag_count));
      if (flags.no_id_keywords_is_fatal)
	{
	  TRY_PRINTF(fprintf(out, "\tid_keywd err/warn\t\n"));
	  ++flag_count;
	}
      if (flags.joint_edit)
	{
	  TRY_PRINTF(fprintf(out, "\tjoint edit\t\n"));
	  ++flag_count;
	}
      if (flags.all_locked)
	{
	  TRY_PRINTF(fprintf(out, "\tlocked releases\ta\n"));
	  ++flag_count;
	}
      else if (!flags.locked.empty())
	{
	  TRY_PRINTF(fprintf(out, "\tlocked releases\t"));
	  TRY_OPERATION(flags.locked.print(out));
	  TRY_PUTC(putc('\n', out));
	  ++flag_count;
	}
      TRY_OPERATION(print_flag(out, "\tmodule\t%s\n", flags.module, flag_count));
      TRY_OPERATION(print_flag(out, "\tnull delta\t\n", flags.null_deltas, flag_count));
      TRY_OPERATION(print_flag(out, "\tcsect name\t%s\n", flags.user_def, flag_count));
      TRY_OPERATION(print_flag(out, "\ttype\t%s\n", flags.type, flag_count));
      TRY_OPERATION(print_flag(out, "\tvalidate MRs\t%s\n", flags.mr_checker, flag_count));

      if (!flags.substitued_flag_letters.empty())
	{
	  ++flag_count;

	  // Unusually, Solaris "prt" just launches into the list of
	  // expanded keyletters, without a leading flag name.
	  TRY_PRINTF(fprintf(out, "\t\t"));
	  TRY_OPERATION(print_subsituted_flags_list(out, " "));
	  TRY_PRINTF(fprintf(out, "\n"));
	}


      if (0 == flag_count)
	{
	  TRY_PRINTF(fprintf(out, "\tnone\n"));
	}
    }
  if (print_desc)
    {
      TRY_PRINTF(fprintf(out, "\nDescription --\n"));
      TRY_OPERATION(print_string_list(out, comments, "\t", "\n", "none"));
      TRY_PUTC(putc('\n', out));
    }

  if (print_body)
    {
      // seek_to_body() is a non-const member, so we have this
      // silly workaround.
      TRY_OPERATION(body_scanner_->print_body(out, name.c_str()));
    }

  return Failure::Ok();
}


/* Local variables: */
/* mode: c++ */
/* End: */
