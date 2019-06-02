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
  TRY_OPERATION(print_flag2(out, (const char *) "branch", flags.branch));
  TRY_OPERATION(print_flag2(out, (const char *) "ceiling", flags.ceiling));
  TRY_OPERATION(print_flag2(out, (const char *) "default SID", flags.default_sid));
  if (flags.encoded)
    {
      TRY_PUTS(fputs("encoded\n", out));
    }
  TRY_OPERATION(print_flag2(out, (const char *) "floor", flags.floor));
  TRY_OPERATION(print_flag2(out, (const char *) "id keywd err/warn",
			    flags.no_id_keywords_is_fatal));
  TRY_OPERATION(print_flag2(out, (const char *) "joint edit", flags.joint_edit));

  const char *locked = "locked releases";
  if (flags.all_locked)
    {
      TRY_OPERATION(print_flag2(out, locked, "a"));
    }
  else
    {
      TRY_OPERATION(print_flag2(out, locked, flags.locked));
    }

  TRY_OPERATION(print_flag2(out, (const char *) "module",
			    (flags.module ? flags.module->c_str()
			     : (const char*)0) ));
  TRY_OPERATION(print_flag2(out, (const char *) "null delta", flags.null_deltas));
  TRY_OPERATION(print_flag2(out, (const char *) "csect name", flags.user_def));
  TRY_OPERATION(print_flag2(out, (const char *) "type", flags.type));
  TRY_OPERATION(print_flag2(out, (const char *) "validate MRs",
			    (flags.mr_checker ? flags.mr_checker->c_str()
			     : (const char*) 0)));

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

#define KEY1(c)         ((unsigned char)(c))
#define KEY2(c1, c2)    (((unsigned char)(c1)) * 256 + (unsigned char)(c2))

/* Prints selected parts of an SCCS file and the specified entry in the
   delta table. */

void
sccs_file::print_delta(FILE *out, const char *outname, const char *format,
                       struct delta const &d)
{
  const char *s = format;

  while (1)
    {
      char c = *s++;

      if (c == '\0')
        {
          break;        // end of format.
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
                   * bit of the format string.  In the latter case we
                   * ignore it - see prs/format.sh test cases 4a and 4b.
                   * Those partiicular test cases were checked against
                   * Sun Solaris 2.6.
                   */
                  if (s[1])
                    {
                      c = '\n';
                      break;
                    }
                  else
                    {
                      return;
                    }
                case 't': c = '\t'; break;
                case '\\': c = '\\'; break;
                default:        // not \n or \t -- print the whole thing.
                  putc('\\', out);
                  c = *s;
                  break;
                }
              putc(c, out);
              ++s;
            }
          else
            {
              putc('\\', out); // trailing backslash at and of format.
            }

          continue;
        }
      else if (c != ':' || s[0] == '\0')
        {
          putc(c, out);
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
          putc(':', out);
          continue;
        }

      // TODO: diagnose write errors in this switch statement.
      switch (key)
        {
        default:
          s = back_to;
          putc(':', out);
          continue;

        case KEY2('D','t'):
          print_delta(out, outname, ":DT: :I: :D: :T: :P: :DS: :DP:", d);
          break;

        case KEY2('D','L'):
          print_delta(out, outname, ":Li:/:Ld:/:Lu:", d);
          break;

        case KEY2('L','i'):
          fprintf(out, "%05lu", d.inserted());
          break;

        case KEY2('L','d'):
          fprintf(out, "%05lu", d.deleted());
          break;

        case KEY2('L','u'):
          fprintf(out, "%05lu", d.unchanged());
          break;

        case KEY2('D','T'):
          putc(d.get_type(), out);
          break;

        case KEY1('I'):
          d.id().print(out);
          break;

        case KEY1('R'):
          d.id().printf(out, 'R');
          break;

        case KEY1('L'):
          d.id().printf(out, 'L');
          break;

        case KEY1('B'):
          d.id().printf(out, 'B');
          break;

        case KEY1('S'):
          d.id().printf(out, 'S');
          break;

        case KEY1('D'):
          d.date().printf(out, 'D');
          break;

        case KEY2('D','y'):
          d.date().printf(out, 'y');
          break;

        case KEY2('D','m'):
          d.date().printf(out, 'o');
          break;

        case KEY2('D','d'):
          d.date().printf(out, 'd');
          break;

        case KEY1('T'):
          d.date().printf(out, 'T');
          break;

        case KEY2('T','h'):
          d.date().printf(out, 'h');
          break;

        case KEY2('T','m'):
          d.date().printf(out, 'm');
          break;

        case KEY2('T','s'):
          d.date().printf(out, 's');
          break;

        case KEY1('P'):
          fputs(d.user().c_str(), out);
          break;

        case KEY2('D','S'):
          fprintf(out, "%u", d.seq());
          break;

        case KEY2('D','P'):
          fprintf(out, "%u", d.prev_seq());
          break;

        case KEY2('D', 'I'):
	  /* Testing with the Solaris 2.6 version only shows one slash (meaning :Dn:/:Dx:),
	     but OpenSolaris 2009.06 (SunOS 5.11) shows two. */
	  if (!d.get_included_seqnos().empty())
	    print_delta(out, outname, ":Dn:", d);
	  if (!d.get_excluded_seqnos().empty())
	    print_delta(out, outname, "/:Dx:", d);
	  if (!d.get_ignored_seqnos().empty())
	    print_delta(out, outname, "/:Dg:", d);
	  break;

        case KEY2('D','n'):
          print_seq_list(out, d.get_included_seqnos());
          break;

        case KEY2('D','x'):
          print_seq_list(out, d.get_excluded_seqnos());
          break;

        case KEY2('D','g'):
          print_seq_list(out, d.get_ignored_seqnos());
	  break;

        case KEY2('M','R'):
          print_string_list(out, d.mrs().cbegin(), d.mrs().cend());
          break;

        case KEY1('C'):
          print_string_list(out, d.comments().cbegin(), d.comments().cend());
          break;

        case KEY2('U','N'):
	  if (!users.empty())
	    print_string_list(out, users.cbegin(), users.cend());
	  else
	    fprintf(out, "%s\n", "none");
          break;

        case KEY2('F', 'L'):
          print_flags(out);
          break;

        case KEY1('Y'):
          print_flag(out, flags.type);
          break;

        case KEY2('M','F'):
          print_yesno(out, flags.mr_checker != 0);
          break;

        case KEY2('M','P'):
          print_flag(out, flags.mr_checker);
          break;

        case KEY2('K','F'):
          print_yesno(out, flags.no_id_keywords_is_fatal);
          break;

        case KEY2('B','F'):
          print_yesno(out, flags.branch);
          break;

        case KEY1('J'):
          print_yesno(out, flags.joint_edit);
          break;

        case KEY2('L','K'):
          if (flags.all_locked)
            {
              putc('a', out);
            }
          else
            {

              if (flags.locked.empty())
                fprintf(out, "none");
              else
                print_flag(out, flags.locked);
            }
          break;

        case KEY1('Q'):
          if (flags.user_def)
            print_flag(out, flags.user_def);
          break;

        case KEY1('M'):
          print_flag(out, get_module_name());
          break;

        case KEY2('F','B'):
          print_flag(out, flags.floor);
          break;

        case KEY2('C','B'):
          print_flag(out, flags.ceiling);
          break;

        case KEY2('D','s'):
          print_flag(out, flags.default_sid);
          break;

        case KEY2('N','D'):
          print_yesno(out, flags.null_deltas);
          break;

        case KEY2('F','D'):
          // The genuine article prints '(none)' if there
          // is no description.
          // JY Sun Nov 25 01:33:46 2001; Solaris 2.6
          // prints "none" rather than "(none)".
          if (comments.empty())
            fputs("none\n", out);
          else
            print_string_list(out, comments.cbegin(), comments.cend());
          break;

        case KEY2('B','D'):
	  if (!body_scanner_->emit_raw_body(out, outname))
	    {
	      /* TODO: signal that something failed.  We already
	         issued an error message, but this function returns
	         void. */
	    }
          break;

        case KEY2('G','B'):
	  {
	    std::string gname = "standard output";
	    struct subst_parms parms(gname, get_module_name(), out,
				     cssc::optional<std::string>(),
				     delta_table->delta_at_seq(d.seq()),
				     0, sccs_date());
	    class seq_state state(highest_delta_seqno());
	    prepare_seqstate(state, d.seq(), sid_list(), sid_list(), sccs_date());
	    do_get(gname, state, parms, true, 0, 0, 0, false, false); // TODO: check return value?
	  }
          break;

        case KEY1('W'):
          print_delta(out, outname, ":Z::M:\t:I:", d);
          break;

        case KEY1('A'):
          print_delta(out, outname, ":Z::Y: :M: :I::Z:", d);
          break;

        case KEY1('Z'):
          fputc('@', out);
          fputs("(#)", out);
          break;

        case KEY1('F'):
          fputs(base_part(name.sfile()).c_str(), out);
          break;

        case KEY2('P','N'):
	  {
	    cssc::FailureOr<std::string> canon = canonify_filename(name.c_str());
	    if (canon.ok())
	      {
		const std::string path(*canon);
		fputs(path.c_str(), out);
	      }
	  }
          break;
        }
    }
}




/* Prints out parts of the SCCS file.  */
bool
sccs_file::prs(FILE *out, const char *outname,
	       const std::string& format, sid rid, sccs_date cutoff_date,
               enum when cutoff_type, delta_selector selector, bool *matched)
{
  const_delta_iterator iter(delta_table.get(), selector);
  const char *fmt = format.c_str();

  if (cutoff_type == SIDONLY)
    {
      ASSERT (!cutoff_date.valid());
      while (iter.next())
	{
	  if (!rid.valid() || (rid == iter->id()))
	    {
	      *matched = true;
	      print_delta(out, outname, fmt, *iter.operator->());
	      putc('\n', out);
	      break;
	    }
	}
    }
  else if (cutoff_type == LATER)
    {
      while (iter.next())
	{
	  if (cutoff_date.valid() && iter->date() < cutoff_date)
	    break;
	  *matched = true;
	  print_delta(out, outname, fmt, *iter.operator->());
	  putc('\n', out);
	  if (rid.valid() && (rid == iter->id()))
	    break;
	}
    }
  else                          // EARLIER
    {
      while (iter.next())
	{
	  if (!*matched)
	    {
	      if (rid.valid() && (rid != iter->id()))
		continue;
	    }
	  if (cutoff_date.valid() && (cutoff_date < iter->date()))
	    continue;
	  *matched = true;
	  print_delta(out, outname, fmt, *iter.operator->());
	  putc('\n', out);
	}
    }
  return true;
}

/* Local variables: */
/* mode: c++ */
/* End: */
