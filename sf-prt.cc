/*
 * sf-prt.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998,1999,2001,2004,2007, 2008 Free Software Foundation, Inc. 
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Members of the class sccs_file for doing sccs-prt.
 */


#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"
#include "filepos.h"
#include "delta.h"
#include "delta-iterator.h"


#ifdef HAVE_UNISTD_H
#include <unistd.h>		// SEEK_SET on SunOS.
#endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-prt.cc,v 1.29 2008/01/05 19:55:30 jay Exp $";
#endif

static void
print_string_list(FILE *out,
		  mylist<mystring> const &l,
		  const char* pre,
		  const char* post,
		  const char* dflt)
{
  const int len = l.length();
  
  if (0 == len)
    {
      fprintf(out, "%s%s", pre, dflt);
    }
  else
    {
      for(int i = 0; i < len; i++)
	{
	  fprintf(out, "%s%s", pre, l[i].c_str());
	  if (i < len-1)
	    {
	      fprintf(out, "%s", post);
	    }
	  
	}
    }
}

void print_flag(FILE *out, const char *fmt, mystring flag, int& count)
{
  if (!flag.empty())
    {
      ++count;
      fprintf(out, fmt, flag.c_str());
    }
}

void print_flag(FILE *out, const char *fmt, const mystring* pflag, int& count)
{
  // We consider a flag which is set to an empty string still to be set.
  // An example is the v flag; lines of the form "^Af v" should still set
  // the v flag to an empty string.
  if (pflag)
    {
      ++count;
      fprintf(out, fmt, pflag->c_str());
    }
}

void print_flag(FILE *out, const char *fmt,  int flag, int& count)
{
  if (flag)
    {
      ++count;
      fprintf(out, fmt, flag ? "yes" : "no");
    }
}

void print_flag(FILE *out, const char *fmt,  sid flag, int& count)
{
  if (flag.valid())
    {
      ++count;
      fprintf(out, "%s", fmt);
      flag.print(out);
      putc('\n', out);
    }
}

void print_flag(FILE *out, const char *fmt,  release flag, int& count)
{
  if (flag.valid())
    {
      ++count;
      fprintf(out, "%s", fmt);
      flag.print(out);
      fprintf(out, "\n");
    }
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
      stop_now = stop_now || (date <= cutoff_delta->date);
      if (date < cutoff_delta->date)
	return true;
    }
  return false;
}

void
sccs_file::cutoff::print(FILE *out) const
{
  fprintf(out, "cutoff: ");
  if (enabled)
    {
      fputs("enabled ", out);
      if (most_recent_sid_only)
	  fputs("most-recent-only ", out);
      fputs("cutoff_sid='", out);
      if (cutoff_sid.valid())
	cutoff_sid.print(out);
      else
	fputs("(invalid)", out);

      fputs("' first_accepted='", out);
      if (first_accepted.valid())
	{
	  first_accepted.printf(out, 'D');
	  fprintf(out, " ");
	  first_accepted.printf(out, 'T');
	}
      else
	{
	fputs("(invalid)'", out);
	}
      fputs("' last_accepted='", out);
      if (last_accepted.valid())
	{
	  last_accepted.printf(out, 'D');
	  fprintf(out, " ");
	  last_accepted.printf(out, 'T');
	}
      else
	{
	fputs("(invalid)'", out);
	}

      if (cutoff_delta)
	{
	  fputs("' cutoff_delta->date='", out);
	  if (cutoff_delta->date.valid())
	    {
	      last_accepted.printf(out, 'D');
	      fprintf(out, " ");
	      last_accepted.printf(out, 'T');
	    }
	  else
	    {
	      fputs("(invalid)'", out);
	    }
	}
      fputs("'\n", out);
    }
  else
    {
      fputs("disabled\n", out);
    }
}

sccs_file::cutoff::cutoff()
  : enabled(false), most_recent_sid_only(false),
    cutoff_sid(sid()),
    cutoff_delta(NULL)
{
  // all done above.
}

// Print the body of an SCCS file, transforming all "^A"s 
// into "*** "s.
static bool 
do_print_body(const char *name, FILE *fp, long body_offset, FILE *out)
{
  bool ret = true;
  
  // When pos_saver goes out of scope the file position on "fp" is restored.
  FilePosSaver pos_saver(fp);

  if (0 != fseek(fp, body_offset, SEEK_SET))
    {
      errormsg_with_errno("%s: fseek() failed!", name);
      return false;		// can't read body now, so just fail.
    }
  
  
  if (putc_failed(putc('\n', out)))
    ret = false;
  
  int ch;
  while ( ret && (ch=getc(fp)) != EOF )
    {
      if ('\001' == ch)
	{
	  if (fputs_failed(fputs("*** ", out)))
	    {
	      ret = false;	// write error
	      break;
	    }
	}
      else if ('\n' == ch)
	{
	  int peek = getc(fp);
	  
	  if ('\001' == peek)
	    {
	      ungetc(peek, fp);
	      if (putc_failed(putc('\n', out)))
		ret = false;
	    }
	  else if (EOF == peek)
	    {
	      if (putc_failed(putc('\n', out)))
		ret = false;
	      break;
	    }
	  else 
	    {
	      ungetc(peek, fp);
	      if (fputs_failed(fputs("\n\t", out)))
		{
		  ret = false;	// write error
		  break;
		}
	    }
	}
      else
	{
	  if (putc_failed(putc(ch, out)))
	    {
	      ret = false;	// write error
	      break;
	    }
	}
    }
  if (ferror(fp))		// read error is fatal.
    {
      errormsg_with_errno("%s: read failed!", name);
      ret = false;
    }
      
  // When pos_saver goes out of scope the file position is restored.
  return ret;
}

static void
print_seq_list(FILE *out, mylist<seq_no> const &list)
{
  const int len = list.length();

  if (len > 0)
    {
      fprintf(out, "%u", list[0]);
      for(int i = 1; i < len; i++)
	{
	  fprintf(out, " %u", list[i]);
	}
    }
}


bool
sccs_file::prt(FILE *out,
	       cutoff exclude,	      // -y, -c, -r
	       int all_deltas,	      // -a
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
      const_delta_iterator iter(delta_table);
  
      while (!stop_now && iter.next(all_deltas))
	{
	  if (exclude.excludes_delta(iter->id, iter->date, stop_now))
	    continue;

	  // Unless -a was specified, don't print removed deltas.
	  if (!all_deltas && iter->removed())
	    continue;
	  
	  if (exclude.enabled)	// -y, -c, or -r option.
	    fprintf(out, "%s:\t", name.c_str());
	  else
	    putc('\n', out);
      
	  // Print the stuff from the delta...
	  fprintf(out, "%c ", iter->type);
	  iter->id.print(out);
	  putc('\t', out);
	  iter->date.printf(out, 'D');
	  putc(' ', out);
	  iter->date.printf(out, 'T');
	  fprintf(out, " %s\t%hu %hu",
		  iter->user.c_str(),
		  (unsigned short)iter->seq,
		  (unsigned short)iter->prev_seq);
	  fprintf(out, "\t%05lu/%05lu/%05lu",
		  iter->inserted, iter->deleted, iter->unchanged);


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
		  if (iter->included.length())
		    {
		      fputs(nl_sep, out);	// either newline or space.
		      fprintf(out, "Included:\t");
		      print_seq_list(out, iter->included);
		    }
		  else if (iter->have_includes)
		    {
		      fputs(nl_sep, out);	// either newline or space.
		      fprintf(out, "Included:\t");
		    }
		  if (iter->excluded.length())
		    {
		      fputs(nl_sep, out);	// either newline or space.
		      fprintf(out, "Excluded:\t");
		      print_seq_list(out, iter->excluded);
		    }
		  else if (iter->have_excludes)
		    {
		      fputs(nl_sep, out);	// either newline or space.
		      fprintf(out, "Excluded:\t");
		    }
		}
	      // Print any MRs and then the comments.
	      if (iter->mrs.length())
		{
		  fputs(nl_sep, out);	// either newline or space.
		  print_string_list(out, iter->mrs, "MRs:\t", nl_sep, "");
		}
	      if (iter->comments.length())
		{
		  fputs(nl_sep, out);	// either newline or space.
		  print_string_list(out, iter->comments, "", nl_sep, "");
		}
	    }
	  putc('\n', out);
	}
    }
      
  // global stuff.
  if (print_users)
    {
      fprintf(out, "\n Users allowed to make deltas -- \n");
      print_string_list(out, users, "\t", "\n", "everyone");
      putc('\n', out);
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
      fprintf(out, "\nFlags --\n");
      if (flags.branch)
	{
	  // "Real" SCCS prints a TAB after "branch", so we do too.
	  fprintf(out, "\tbranch\t\n");
	  ++flag_count;
	}
      
      print_flag(out, "\tceiling\t", flags.ceiling, flag_count);
      print_flag(out, "\tdefault SID\t", flags.default_sid, flag_count);

      // No newline after this one;  odd, but it's what "real"
      // SCCS does.
      if (flags.encoded)
	{
	  fprintf(out, "\tencoded");
	  ++flag_count;
	}

#if 0
      // The 'x' flag is a SCO OpenServer extension.
      // SCO OpenServer 5.0.6 has no "prt" command, but
      // SCO Unixware 8.0.0 does have it.  However, it 
      // does not print anything if the x flag is set.  
      // Hence for compatibility, we don't do that either.
      print_flag(out, "\texecutable\t\n", flags.executable, flag_count);
#endif
      
      print_flag(out, "\tfloor\t", flags.floor, flag_count);
      if (flags.no_id_keywords_is_fatal)
	{
	  fprintf(out, "\tid_keywd err/warn\t\n");
	  ++flag_count;
	}
      if (flags.joint_edit)
	{
	  fprintf(out, "\tjoint edit\t\n");
	  ++flag_count;
	}
      if (flags.all_locked)
	{
	  fprintf(out, "\tlocked releases\ta\n");
	  ++flag_count;
	}
      else if (!flags.locked.empty())
	{
	  fprintf(out, "\tlocked releases\t");
	  flags.locked.print(out);
	  putc('\n', out);
	  ++flag_count;
	}
      print_flag(out, "\tmodule\t%s\n", flags.module, flag_count);
      print_flag(out, "\tnull delta\t\n", flags.null_deltas, flag_count);
      print_flag(out, "\tcsect name\t%s\n", flags.user_def, flag_count);
      print_flag(out, "\ttype\t%s\n", flags.type, flag_count);
      print_flag(out, "\tvalidate MRs\t%s\n", flags.mr_checker, flag_count);

      if (flags.substitued_flag_letters.count() > 0)
	{
	  ++flag_count;

	  // Unusually, Solaris "prt" just launches into the list of 
	  // expanded keyletters, without a leading flag name.
	  fprintf(out, "\t\t");
	  (void) print_subsituted_flags_list(out, " ");
	  fprintf(out, "\n");
	}
      
      
      if (0 == flag_count)
	fprintf(out, "\tnone\n");
    }
  if (print_desc)
    {
      fprintf(out, "\nDescription --\n");
      print_string_list(out, comments, "\t", "\n", "none");
      putc('\n', out);
    }

  if (print_body)
    {
      // seek_to_body() is a non-const member, so we have this
      // silly workaround.
      do_print_body(name.c_str(), f, body_offset, out);
    }
  
  return true;
}


/* Local variables: */
/* mode: c++ */
/* End: */
