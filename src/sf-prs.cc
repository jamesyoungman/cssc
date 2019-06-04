/*
 * sf-prs.cc: Part of GNU CSSC.
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
 * Members of the class sccs_file for printing selected parts of an
 * SCCS file.
 *
 */

#include <config.h>

#include "cssc.h"
#include "failure.h"
#include "failure_macros.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta.h"
#include "delta-iterator.h"
#include "delta-table.h"
#include "linebuf.h"
#include "cssc-assert.h"
#include "subst-parms.h"

using cssc::Failure;
using cssc::FailureOr;
using cssc::make_failure_from_errno;

/* Prints a list of sequence numbers on the same line. */
static Failure
print_seq_list(FILE *out, std::vector<seq_no> const &list) {
  const std::vector<seq_no>::size_type len = list.size();
  // TODO: see if we can use a more natural STL-like construct here.
  /* prs does actually print the sequences in reverse order! */
  if (len > 0)
    {
      std::vector<seq_no>::size_type i = len-1;
      do
	{
	  TRY_PRINTF(fprintf(out, "%u", list[i]));
	  if (i > 0)
	    {
	      TRY_PRINTF(fprintf(out, " "));
	    }
	} while (i--);
    }
  return Failure::Ok();
}


/* Prints a list of strings, one per line. */
template <class InputIterator>
static Failure
print_string_list(FILE *out, InputIterator first, InputIterator last)
{
  for (InputIterator it = first; it != last; ++it)
    {
      TRY_PRINTF(fprintf(out, "%s\n", it->c_str()));
    }
  return Failure::Ok();
}

/* Prints a boolean flag with its name.   Simply, if the
 * flag is unset, its name is not printed.
 */
static Failure
print_flag2(FILE *out, const char *s, int it)
{
  if (it)
    {
      TRY_PRINTF(fprintf(out, "%s\n", s));
    }
  return Failure::Ok();
}


/* Prints a flag whose type has a print(FILE *) member with its name. */

Failure
print_flag2(FILE *out, const char *s, const sid& it)
{
  if (it.valid())
    {
      TRY_PRINTF(fprintf(out, "%s\t", s));
      TRY_OPERATION(it.print(out));
      TRY_PUTC(putc('\n', out));
    }
  return Failure::Ok();
}

Failure
print_flag2(FILE *out, const char *s, const release_list& it)
{
  if (it.valid())
    {
      TRY_PRINTF(fprintf(out, "%s\t", s));
      TRY_OPERATION(it.print(out));
      TRY_PUTC(putc('\n', out));
    }
  return Failure::Ok();
}

Failure
print_flag2(FILE *out, const char *s, const release& it)
{
  if (it.valid())
    {
      TRY_PRINTF(fprintf(out, "%s\t", s));
      TRY_OPERATION(it.print(out));
      TRY_PUTC(putc('\n', out));
    }
  return Failure::Ok();
}

static inline Failure
print_flag2(FILE *out, const char *name, const std::string *s)
{
  if (s)
    {
      TRY_PRINTF(fprintf(out, "%s\t%s\n", name, s->c_str()));
    }
  return Failure::Ok();
}

static inline Failure
print_flag2(FILE *out, const char *name, const char *s)
{
  if (s)
    {
      TRY_PRINTF(fprintf(out, "%s\t%s\n", name, s));
    }
  return Failure::Ok();
}

static inline Failure
print_flag2(FILE *out, const char *name, char *s)
{
  if (s)
    {
      TRY_PRINTF(fprintf(out, "%s\t%s\n", name, s));
    }
  return Failure::Ok();
}

/* Prints all the flags of an SCCS file. */


Failure
sccs_file::print_flags(FILE *out) const
{
  TRY_OPERATION(print_flag2(out, "branch", flags.branch));
  TRY_OPERATION(print_flag2(out, "ceiling", flags.ceiling));
  TRY_OPERATION(print_flag2(out, "default SID", flags.default_sid));
  if (flags.encoded)
    {
      TRY_PUTS(fputs("encoded\n", out));
    }
  TRY_OPERATION(print_flag2(out, "floor", flags.floor));
  TRY_OPERATION(print_flag2(out, "id keywd err/warn",
			    flags.no_id_keywords_is_fatal));
  TRY_OPERATION(print_flag2(out, "joint edit", flags.joint_edit));

  const char *locked = "locked releases";
  if (flags.all_locked)
    {
      TRY_OPERATION(print_flag2(out, locked, "a"));
    }
  else
    {
      TRY_OPERATION(print_flag2(out, locked, flags.locked));
    }

  TRY_OPERATION(print_flag2(out, "module",
			    (flags.module ? flags.module->c_str() : nullptr) ));
  TRY_OPERATION(print_flag2(out, "null delta", flags.null_deltas));
  TRY_OPERATION(print_flag2(out, "csect name", flags.user_def));
  TRY_OPERATION(print_flag2(out, "type", flags.type));
  TRY_OPERATION(print_flag2(out, "validate MRs",
			    (flags.mr_checker ? flags.mr_checker->c_str() : nullptr)));

#if 0
  // Testing on Solaris 9 reveals that no output is produced
  // if the "y" flag is set.  Hence for compatibility we also
  // say nothing.
  if (flags.substitued_flag_letters.count() > 0)
    {
      TRY_PUTS(fputs("substituted keywords\t", out));
      TRY_OPERATION(print_subsituted_flags_list(out, " "));
      TRY_PUTS(fputs("\n", out));
    }
#endif
  return Failure::Ok();
}


/* Prints "yes" or "no" according to the value of a boolean flag. */

inline static Failure
print_yesno(FILE *out, int flag)
{
  const char * representation = flag ? "yes" : "no";
  TRY_PUTS(fputs(representation, out));
  return Failure::Ok();
}

/* Prints the value of a std::string flag. */
inline static Failure
print_flag(FILE *out, const std::string *s)
{
  const char * representation = (s == nullptr) ? "none" : s->c_str();
  TRY_PUTS(fputs(representation, out));
  return Failure::Ok();
}

/* Prints the value of a std::string flag. */
inline static Failure
print_flag(FILE *out, const std::string &s)
{
  const char * representation = s.empty() ? "none" : s.c_str();
  TRY_PUTS(fputs(representation, out));
  return Failure::Ok();
}


inline static Failure
print_flag(FILE *out, const release_list &it)
{
  if (it.valid())
    {
      TRY_OPERATION(it.print(out));
    }
  else
    {
      TRY_PUTS(fputs("none", out));
    }
  return Failure::Ok();
}

inline static Failure
print_flag(FILE *out, const release &it)
{
  if (it.valid())
    {
      TRY_OPERATION(it.print(out));
    }
  else
    {
      TRY_PUTS(fputs("none", out));
    }
  return Failure::Ok();
}

inline static Failure
print_flag(FILE *out, const sid &it)
{
  if (it.valid())
    {
      TRY_OPERATION(it.print(out));
    }
  else
    {
      TRY_PUTS(fputs("none", out));
    }
  return Failure::Ok();
}

// /* Prints the value of string flag. */
// template <class TYPE>
// void
// print_flag(FILE *out, TYPE it)
// {
//   if (it.valid())
//     it.print(out);
//   else
//     fputs("none", out);
// }

/* These macros are used to convert the one or two characters a prs
   data keyword in an unsigned value used in the switch statement
   in the function below. */

#define KEY1(c)         (static_cast<unsigned char>(c))
#define KEY2(c1, c2)    ((static_cast<unsigned char>(c1)) * 256 + static_cast<unsigned char>(c2))

/* Prints selected parts of an SCCS file and the specified entry in the
   delta table. */

Failure
sccs_file::print_delta(FILE *out, const char *outname, const char *format,
                       struct delta const &d)
{
  const char *s = format;

  while (1)
    {
      char c = *s++;

      if (c == '\0')
        {
	  // end of format.
	  return Failure::Ok();
        }
      else if ('\\' == c)
        {
          if ('\0' != *s)
            {
              // Not at the end of the format string.
              // Backslash escape codes.  We only recognise \n and \t.
              switch (*s)
                {
                case 'n':
                  /* Turn a \n into a newline unless it is the last
                   * bit of the format string. */
                  if (s[1])
                    {
                      c = '\n';
                      break;
                    }
                  else
                    {
		      /* The \n is the last bit of the format string.
		       * In this case we ignore it - see prs/format.sh
		       * test cases 4a and 4b.  Those partiicular test
		       * cases were checked against Sun Solaris 2.6.
		       */
                      return Failure::Ok();
                    }
                case 't': c = '\t'; break;
                case '\\': c = '\\'; break;
                default:        // not \n or \t -- print the whole thing.
		  if (fputc_failed(putc('\\', out)))
		    return make_failure_from_errno(errno);
                  c = *s;
                  break;
                }
	      if (fputc_failed(putc(c, out)))
		return make_failure_from_errno(errno);
              ++s;
            }
          else
            {
	      // trailing backslash at and of format.
	      if (fputc_failed(putc('\\', out)))
		return make_failure_from_errno(errno);
            }

          continue;
        }
      else if (c != ':' || s[0] == '\0')
        {
	  if (fputc_failed(putc(c, out)))
	    return make_failure_from_errno(errno);
	  else
	    continue;
        }

      const char *back_to = s;
      unsigned key = 0;

      if (s[1] == ':')
        {
          key = KEY1(s[0]);
          s += 2;
        }
      else if (s[2] == ':')
        {
          key = KEY2(s[0], s[1]);
          s += 3;
        }
      else
        {
	  if (fputc_failed(putc(':', out)))
	    return make_failure_from_errno(errno);
	  else
	    continue;
        }
      cssc::FailureOr<bool> fail_or_recognised = print_delta_key(out, outname, key, d);
      if (!fail_or_recognised.ok())
	return fail_or_recognised.fail();
      if (!*fail_or_recognised)
	{
	  s = back_to;
	  putc(':', out);
	  continue;
	}
    }
}


cssc::FailureOr<bool>
sccs_file::print_delta_key(FILE *out_file,
			   const char *out_file_name,
			   unsigned combined_key,
			   struct delta const &dd)
{
  bool recognised = true;
  auto do_print_delta_key = [this, &recognised](FILE *out,
						const char *outname,
						unsigned key,
						struct delta const &d) -> Failure
    {
      switch (key)
	{
	case KEY2('D','t'):
	return print_delta(out, outname, ":DT: :I: :D: :T: :P: :DS: :DP:", d);

	case KEY2('D','L'):
	return print_delta(out, outname, ":Li:/:Ld:/:Lu:", d);

	case KEY2('L','i'):
	return fprintf_failure(fprintf(out, "%05lu", d.inserted()));

	case KEY2('L','d'):
	return fprintf_failure(fprintf(out, "%05lu", d.deleted()));

	case KEY2('L','u'):
	return fprintf_failure(fprintf(out, "%05lu", d.unchanged()));

	case KEY2('D','T'):
	return fputc_failure(d.get_type(), out);

	case KEY1('I'):
	return d.id().print(out);

	case KEY1('R'):
	return d.id().printf(out, 'R');

	case KEY1('L'):
	return d.id().printf(out, 'L');

	case KEY1('B'):
	return d.id().printf(out, 'B');

	case KEY1('S'):
	return d.id().printf(out, 'S');

	case KEY1('D'):
	return d.date().printf(out, 'D');

	case KEY2('D','y'):
	return d.date().printf(out, 'y');

	case KEY2('D','m'):
	return d.date().printf(out, 'o');

	case KEY2('D','d'):
	return d.date().printf(out, 'd');

	case KEY1('T'):
	return d.date().printf(out, 'T');

	case KEY2('T','h'):
	return d.date().printf(out, 'h');

	case KEY2('T','m'):
	return d.date().printf(out, 'm');

	case KEY2('T','s'):
	return d.date().printf(out, 's');

	case KEY1('P'):
	if (fputs_failed(fputs(d.user().c_str(), out)))
	  {
	    return make_failure_from_errno(errno);
	  }
	return Failure::Ok();

	case KEY2('D','S'):
	return fprintf_failure(fprintf(out, "%u", d.seq()));

	case KEY2('D','P'):
	return fprintf_failure(fprintf(out, "%u", d.prev_seq()));

	case KEY2('D', 'I'):
	/* Testing with the Solaris 2.6 version only shows one slash (meaning :Dn:/:Dx:),
	   but OpenSolaris 2009.06 (SunOS 5.11) shows two. */
	{
	  Failure done = Failure::Ok();
	  if (!d.get_included_seqnos().empty())
	    done = print_delta(out, outname, ":Dn:", d);
	  if (done.ok() && !d.get_excluded_seqnos().empty())
	    done = print_delta(out, outname, "/:Dx:", d);
	  if (done.ok() && !d.get_ignored_seqnos().empty())
	    done = print_delta(out, outname, "/:Dg:", d);
	  return done;
	}

	case KEY2('D','n'):
	return print_seq_list(out, d.get_included_seqnos());

	case KEY2('D','x'):
	return print_seq_list(out, d.get_excluded_seqnos());

	case KEY2('D','g'):
	return print_seq_list(out, d.get_ignored_seqnos());

	case KEY2('M','R'):
	return print_string_list(out, d.mrs().cbegin(), d.mrs().cend());

	case KEY1('C'):
	return print_string_list(out, d.comments().cbegin(), d.comments().cend());

	case KEY2('U','N'):
	if (!users.empty())
	  return print_string_list(out, users.cbegin(), users.cend());
	else
	  return fprintf_failure(fprintf(out, "%s\n", "none"));

	case KEY2('F', 'L'):
	return print_flags(out);

	case KEY1('Y'):
	return print_flag(out, flags.type);

	case KEY2('M','F'):
	return print_yesno(out, flags.mr_checker != nullptr);

	case KEY2('M','P'):
	return print_flag(out, flags.mr_checker);

	case KEY2('K','F'):
	return print_yesno(out, flags.no_id_keywords_is_fatal);

	case KEY2('B','F'):
	return print_yesno(out, flags.branch);

	case KEY1('J'):
	return print_yesno(out, flags.joint_edit);

	case KEY2('L','K'):
	if (flags.all_locked)
	  {
	    return fputc_failure('a', out);
	  }
	else
	  {
	    if (flags.locked.empty())
	      {
		return fprintf_failure(fprintf(out, "none"));
	      }
	    else
	      {
		return print_flag(out, flags.locked);
	      }
	  }

	case KEY1('Q'):
	if (flags.user_def)
	  return print_flag(out, flags.user_def);
	else
	  return Failure::Ok();

	case KEY1('M'):
	return print_flag(out, get_module_name());

	case KEY2('F','B'):
	return print_flag(out, flags.floor);

	case KEY2('C','B'):
	return print_flag(out, flags.ceiling);

	case KEY2('D','s'):
	return print_flag(out, flags.default_sid);

	case KEY2('N','D'):
	return print_yesno(out, flags.null_deltas);

	case KEY2('F','D'):
	// The genuine article prints '(none)' if there
	// is no description.
	// JY Sun Nov 25 01:33:46 2001; Solaris 2.6
	// prints "none" rather than "(none)".
	if (comments.empty())
	  {
	    if (fputs_failed(fputs("none\n", out)))
	      return make_failure_from_errno(errno);
	    else
	      return Failure::Ok();
	  }
	else
	  {
	    return print_string_list(out, comments.cbegin(), comments.cend());
	  }

	case KEY2('B','D'):
	return body_scanner_->emit_raw_body(out, outname);

	case KEY2('G','B'):
	{
	  std::string gname = "standard output";
	  struct subst_parms parms(gname, get_module_name(), out,
				   cssc::optional<std::string>(),
				   delta_table->delta_at_seq(d.seq()),
				   0, sccs_date());
	  class seq_state state(highest_delta_seqno());
	  prepare_seqstate(state, d.seq(), sid_list(), sid_list(), sccs_date());
	  return do_get(gname, state, parms, true, 0, 0, 0, false, false);
	}

	case KEY1('W'):
	return print_delta(out, outname, ":Z::M:\t:I:", d);

	case KEY1('A'):
	return print_delta(out, outname, ":Z::Y: :M: :I::Z:", d);

	case KEY1('Z'):
	if (fputc_failed(fputc('@', out)) || fputs_failed(fputs("(#)", out)))
	  return make_failure_from_errno(errno);
	return Failure::Ok();
	break;

	case KEY1('F'):
	if (fputs_failed(fputs(base_part(name.sfile()).c_str(), out)))
	  {
	    return make_failure_from_errno(errno);
	  }
	return Failure::Ok();

	case KEY2('P','N'):
	{
	  cssc::FailureOr<std::string> canon = canonify_filename(name.c_str());
	  if (!canon.ok())
	    return canon.fail();
	  const std::string path(*canon);
	  if (fputs_failed(fputs(path.c_str(), out)))
	    {
	      return make_failure_from_errno(errno);
	    }
	  return Failure::Ok();
	}
	}
      recognised = false;
      return Failure::Ok();
    };

  Failure printed = do_print_delta_key(out_file, out_file_name, combined_key, dd);
  if (!printed.ok())
    {
      return printed;
    }
  else
    {
      return recognised;
    }
}


/* Prints out parts of the SCCS file.  */
cssc::FailureOr<bool>
sccs_file::prs(FILE *out, const char *outname,
	       const std::string& format, sid rid, sccs_date cutoff_date,
               enum when cutoff_type, delta_selector selector)
{
  const_delta_iterator iter(delta_table.get(), selector);
  const char *fmt = format.c_str();
  bool matched = false;

  if (cutoff_type == when::SIDONLY)
    {
      ASSERT (!cutoff_date.valid());
      while (iter.next())
	{
	  if (!rid.valid() || (rid == iter->id()))
	    {
	      matched = true;
	      Failure printed = print_delta(out, outname, fmt, *iter.operator->());
	      if (!printed.ok())
		return printed;
	      if (fputc_failed(putc('\n', out)))
		return make_failure_from_errno(errno);
	      break;
	    }
	}
    }
  else if (cutoff_type == when::LATER)
    {
      while (iter.next())
	{
	  if (cutoff_date.valid() && iter->date() < cutoff_date)
	    break;
	  matched = true;
	  Failure printed = print_delta(out, outname, fmt, *iter.operator->());
	  if (!printed.ok())
	    return printed;
	  if (fputc_failed(putc('\n', out)))
	    return make_failure_from_errno(errno);
	  if (rid.valid() && (rid == iter->id()))
	    break;
	}
    }
  else                          // EARLIER
    {
      while (iter.next())
	{
	  if (!matched)
	    {
	      if (rid.valid() && (rid != iter->id()))
		continue;
	    }
	  if (cutoff_date.valid() && (cutoff_date < iter->date()))
	    continue;
	  matched = true;
	  Failure printed = print_delta(out, outname, fmt, *iter.operator->());
	  if (!printed.ok())
	    return printed;
	  if (fputc_failed(putc('\n', out)))
	    return make_failure_from_errno(errno);
	}
    }
  return matched;
}

/* Local variables: */
/* mode: c++ */
/* End: */
