/*
 * sf-prs.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999,2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of the class sccs_file for printing selected parts of an
 * SCCS file.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "delta.h"
#include "delta-iterator.h"
#include "linebuf.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-prs.cc,v 1.30 2002/03/10 17:54:57 james_youngman Exp $";
#endif

inline bool
sccs_file::get(FILE *out, mystring name, seq_no seq)
{
  struct subst_parms parms(out, NULL, delta(), 0,
			   sccs_date());  // XXX: was sccs_date(NULL) (bad!)
  class seq_state state(highest_delta_seqno());
  
  return prepare_seqstate(state, seq) && get(name, state, parms, true);
}

/* Prints a list of sequence numbers on the same line. */

static void
print_seq_list(FILE *out, mylist<seq_no> const &list) {
        int i;
        int len = list.length();

        if (len > 0) {
                fprintf(out, "%u", list[0]);
                for(i = 1; i < len; i++) {
                        fprintf(out, " %u", list[i]);
                }
        }
}


/* Prints a list of strings, one per line. */

static void
print_string_list(FILE *out, mylist<mystring> const &list) {
        int i;
        int len = list.length();

        for(i = 0; i < len; i++) {
                fprintf(out, "%s\n", list[i].c_str());
        }
}


/* Prints a boolean flag with its name.   Simply, if the 
 * flag is unset, its name is not printed.
 */
static void
print_flag2(FILE *out, const char *s, int it)
{
  if (it)
    fprintf(out, "%s\n", s);
}


/* Prints a flag whose type has a print(FILE *) member with it's name. */

template <class TYPE>
void
print_flag2(FILE *out, const char *s, TYPE it) {
        if (it.valid()) {
                fprintf(out, "%s\t", s);
                it.print(out);
                putc('\n', out);
        }
}

// /* Prints a string flag with its name.
//  */
// static void
// print_flag2(FILE *out, const char *s, mystring it)
// {
//   if (!it.empty())
//     fprintf(out, "%s: %s\n", s, it.c_str());
// }


static inline void
print_flag2(FILE *out, const char *name, mystring *s)
{
  if (s)
    fprintf(out, "%s\t%s\n", name, s->c_str());
}

static inline void
print_flag2(FILE *out, const char *name, const char *s)
{
  if (s)
    fprintf(out, "%s\t%s\n", name, s);
}

static inline void
print_flag2(FILE *out, const char *name, char *s)
{
  if (s)
    fprintf(out, "%s\t%s\n", name, s);
}

/* Prints all the flags of an SCCS file. */


void
sccs_file::print_flags(FILE *out) const
{
  print_flag2(out, (const char *) "branch", flags.branch);
  print_flag2(out, (const char *) "ceiling", flags.ceiling);
  print_flag2(out, (const char *) "default SID", flags.default_sid);
  if (flags.encoded) fputs("encoded\n", out);
  print_flag2(out, (const char *) "floor", flags.floor);
  print_flag2(out, (const char *) "id keywd err/warn",
              flags.no_id_keywords_is_fatal);
  print_flag2(out, (const char *) "joint edit", flags.joint_edit);

  const char *locked = "locked releases";
  if (flags.all_locked)
    print_flag2(out, locked, "a");
  else
    print_flag2(out, locked, flags.locked);
  
  print_flag2(out, (const char *) "module",
              (flags.module ? flags.module->c_str()
               : (const char*)0) );
  print_flag2(out, (const char *) "null delta", flags.null_deltas);
  print_flag2(out, (const char *) "csect name", flags.user_def);
  print_flag2(out, (const char *) "type", flags.type);
  print_flag2(out, (const char *) "validate MRs",
              (flags.mr_checker ? flags.mr_checker->c_str()
               : (const char*) 0));
}


/* Prints "yes" or "no" according to the value of a boolean flag. */

inline static void
print_yesno(FILE *out, int flag) {
        if (flag) {
                fputs("yes", out);
        } else {
                fputs("no", out);
        }
}

/* Prints the value of a mystring flag. */
inline static void
print_flag(FILE *out, const mystring *s)
{
  if (s) 
    fputs("none", out);
  else
    fputs(s->c_str(), out);
}

/* Prints the value of a mystring flag. */
inline static void
print_flag(FILE *out, mystring *s)
{
  if (s) 
    fputs(s->c_str(), out);
  else
    fputs("none", out);
}

/* Prints the value of a mystring flag. */
inline static void
print_flag(FILE *out, const mystring &s)
{
  if (s.empty()) 
    fputs("none", out);
  else
    fputs(s.c_str(), out);
}


inline static void
print_flag(FILE *out, const release_list &it)
{
  if (it.valid()) 
    it.print(out);
  else
    fputs("none", out);
}

inline static void
print_flag(FILE *out, const release &it)
{
  if (it.valid()) 
    it.print(out);
  else
    fputs("none", out);
}

inline static void
print_flag(FILE *out, const sid &it)
{
  if (it.valid()) 
    it.print(out);
  else
    fputs("none", out);
}

// /* Prints the the value of string flag. */
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
sccs_file::print_delta(FILE *out, const char *format,
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

      switch (key)
        {
        default:
          s = back_to;
          putc(':', out);
          continue;
                        
        case KEY2('D','t'):
          print_delta(out, ":DT: :I: :D: :T: :P: :DS: :DP:",
                      d);
          break;

        case KEY2('D','L'):
          print_delta(out, ":Li:/:Ld:/:Lu:", d);
          break;
          
        case KEY2('L','i'):
          fprintf(out, "%05lu", d.inserted);
          break;

        case KEY2('L','d'):
          fprintf(out, "%05lu", d.deleted);
          break;

        case KEY2('L','u'):
          fprintf(out, "%05lu", d.unchanged);
          break;

        case KEY2('D','T'):
          putc(d.type, out);
          break;

        case KEY1('I'):
          d.id.print(out);
          break;

        case KEY1('R'):
          d.id.printf(out, 'R');
          break;

        case KEY1('L'):
          d.id.printf(out, 'L');
          break;
          
        case KEY1('B'):
          d.id.printf(out, 'B');
          break;
          
        case KEY1('S'):
          d.id.printf(out, 'S');
          break;
          
        case KEY1('D'):
          d.date.printf(out, 'D');
          break;
          
        case KEY2('D','y'):
          d.date.printf(out, 'y');
          break;

        case KEY2('D','m'):
          d.date.printf(out, 'o');
          break;
          
        case KEY2('D','d'):
          d.date.printf(out, 'd');
          break;
          
        case KEY1('T'):
          d.date.printf(out, 'T');
          break;
          
        case KEY2('T','h'):
          d.date.printf(out, 'h');
          break;
          
        case KEY2('T','m'):
          d.date.printf(out, 'm');
          break;

        case KEY2('T','s'):
          d.date.printf(out, 's');
          break;

        case KEY1('P'):
          fputs(d.user.c_str(), out);
          break;

        case KEY2('D','S'):
          fprintf(out, "%u", d.seq);
          break;

        case KEY2('D','P'):
          fprintf(out, "%u", d.prev_seq);
          break;

        case KEY2('D', 'I'):
          if (d.included.length() > 0 ||
              d.excluded.length() > 0 ||
              d.ignored.length()  > 0   )
            {
              print_delta(out, ":Dn:/:Dx:/:Dg:", d);
              break;
            }
                  
        case KEY2('D','n'):
          print_seq_list(out, d.included);
          break;

        case KEY2('D','x'):
          print_seq_list(out, d.excluded);
          break;

        case KEY2('D','g'):
          print_seq_list(out, d.ignored);
                        break;

        case KEY2('M','R'):
          print_string_list(out, d.mrs);
          break;

        case KEY1('C'):
          print_string_list(out, d.comments);
          break;

        case KEY2('U','N'):
          print_string_list(out, users);
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
          if (0 == comments.length())
            fputs("none\n", out);
          else
            print_string_list(out, comments);
          break;

        case KEY2('B','D'):
          if (seek_to_body())
            {
              while (read_line() != -1)
                {
                  fputs(plinebuf->c_str(), out);
                  putc('\n', out);
                }
            }
          else
            {
              // TODO: what should we do if the seek fails?
              // do nothing.
            }
          break;

        case KEY2('G','B'):
          get(out, "-", d.seq); // TODO: check return value?
          break;

        case KEY1('W'):
          print_delta(out, ":Z::M:\t:I:", d);
          break;

        case KEY1('A'):
          print_delta(out, ":Z::Y: :M: :I::Z:", d);
          break;

        case KEY1('Z'):
          fputc('@', out);
          fputs("(#)", out);
          break;

        case KEY1('F'):
          fputs(base_part(name.sfile()).c_str(), out);
          break;

        case KEY2('P','N'):
          fputs(canonify_filename(name.c_str()).c_str(), out);
          break;
        }
    }
}


/* Prints out parts of the SCCS file.  */

bool
sccs_file::prs(FILE *out, mystring format, sid rid, sccs_date cutoff_date,
               enum when when, int all_deltas)
{
  if (!rid.valid())
    {
      rid = find_most_recent_sid(rid);
    }

  if (when != SIDONLY && !cutoff_date.valid())
    {
      const delta *pd = find_delta(rid);
      if (0 == pd)
        {
          errormsg("%s: Requested SID doesn't exist.", name.c_str());
          return false;
        }
      cutoff_date = pd->date;
    }

  const_delta_iterator iter(delta_table);
  while (iter.next(all_deltas))
    {
      switch (when)
        {
        case EARLIER:
          if (iter->date > cutoff_date)
            {
              continue;
            }
          break;

        case SIDONLY:
          if (rid != iter->id)
            {
              continue;
            }
          break;

        case LATER:
          if (iter->date < cutoff_date)
            {
              continue;
            }
          break;
        }

      print_delta(out, format.c_str(), *iter.operator->());
      putc('\n', out);
    }
  
  if (ferror(out))
    {
      errormsg("%s: Ouput file error.", name.c_str());
      return false;
    }
  return true;
}

// Explicit template instantiations.
template void print_flag2(FILE *out, const char *s, release);
template void print_flag2(FILE *out, const char *s, sid);
template void print_flag2(FILE *out, const char *s, release_list);

/* Local variables: */
/* mode: c++ */
/* End: */
