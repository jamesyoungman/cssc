/*
 * sf-prt.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997  Free Software Foundation, Inc.
 *                     675 Mass Ave, Cambridge, MA 02139, USA
 *
 * Members of the class sccs_file for doing sccs-prt.
 *
 */


#include "cssc.h"
#include "sccsfile.h"
#include "seqstate.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-prt.cc,v 1.4 1997/06/01 00:30:54 james Exp $";
#endif

static void
print_string_list(FILE *out,
		  list<mystring> const &list,
		  mystring pre,
		  mystring post,
		  mystring dflt)
{
  const int len = list.length();
  if (0 == len)
    {
      fprintf(out, "%s%s%s",
	      (const char *)pre,
	      (const char *)dflt,
	      (const char *)post);
    }
  else
    {
      for(int i = 0; i < len; i++)
	{
	  fprintf(out, "%s%s%s",
		  (const char *)pre,
		  (const char *)list[i],
		  (const char *)post);
	}
    }
}

void print_flag(FILE *out, const char *fmt,  mystring flag, int& count)
{
  const char *s = (const char *)flag;
  if (s && strlen(flag))
    {
      ++count;
      fprintf(out, fmt, s);
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

bool sccs_file::cutoff::excludes_delta(sid s,
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
      // Don't set stop_now, since we have not eve got to the first
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
  fpos_t saved_pos;

  if (0 != fgetpos(fp, &saved_pos))
    quit(errno, "%s: fgetpos() failed!", name);
  
  if (0 != fseek(fp, body_offset, SEEK_SET))
    quit(errno, "%s: fseek() failed!", name);
  
  int ch;
  bool ret = true;
  
  if (EOF == putc('\n', out))
    ret = false;
  
  while ( ret && (ch=getc(fp)) != EOF )
    {
      if (ferror(fp))		// read error is fatal.
	quit(errno, "%s: read failed!", name);
      
      if ('\001' == ch)
	{
	  if (EOF == fputs("*** ", out))
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
	      if (EOF == putc('\n', out))
		ret = false;
	    }
	  else if (EOF == peek)
	    {
	      if (EOF == putc('\n', out))
		ret = false;
	      break;
	    }
	  else 
	    {
	      ungetc(peek, fp);
	      if (EOF == fputs("\n\t", out))
		{
		  ret = false;	// write error
		  break;
		}
	    }
	}
      else
	{
	  if (EOF == putc(ch, out))
	    {
	      ret = false;	// write error
	      break;
	    }
	}
    }
  if (0 != fsetpos(fp, &saved_pos))
    quit(errno, "%s: fsetpos() failed!", name);
  return ret;
}

static void
print_seq_list(FILE *out, list<seq_no> const &list)
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


void		
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
  if (print_body)
    {
      print_delta_table = 0;
    }
  
  if (print_delta_table)
    {
      if (exclude.enabled)
	{
	  // exclude.print(stdout);
	  if (exclude.most_recent_sid_only)
	    {
	      find_most_recent_sid(exclude.cutoff_sid,
				   exclude.first_accepted);
	    }
	  if (exclude.cutoff_sid.valid())
	    exclude.cutoff_delta = delta_table.find(exclude.cutoff_sid);
  
	  // exclude.print(stdout);
	}
      
      bool stop_now = false;
      delta_iterator iter(delta_table);
  
      while(!stop_now && iter.next(all_deltas))
	{
	  if (exclude.excludes_delta(iter->id, iter->date, stop_now))
	    continue;

	  // Unless -a was specified, don't print removed deltas.
	  if (!all_deltas && 'R' == iter->type)
	    continue;
	  
	  if (exclude.enabled)	// -y, -c, or -r option.
	    fprintf(out, "%s:\t", (const char *)name);
	  else
	    fprintf(out, "\n");
      
	  // Print the stuff from the delta...
	  fprintf(out, "%c ", iter->type);
	  iter->id.print(out);
	  fprintf(out, "\t");
	  iter->date.printf(out, 'D');
	  fprintf(out, " ");
	  iter->date.printf(out, 'T');
	  fprintf(out, " %s\t%hu %hu",
		  (const char*)iter->user,
		  (unsigned short)iter->seq,
		  (unsigned short)iter->prev_seq);
	  fprintf(out, "\t%05u/%05u/%05u\n",
		  iter->inserted, iter->deleted, iter->unchanged);

	  if (!first_line_only)
	    {
	      if (incl_excl_ignore)
		{
		  fprintf(out, "Included:\t");
		  print_seq_list(out, iter->included);
		  fprintf(out, "\nExcluded:\t");
		  print_seq_list(out, iter->excluded);
		  fprintf(out, "\n");
		}
	      // Print any MRs and then the comments.
	      if (iter->mrs.length())
		print_string_list(out, iter->mrs, "MRs:\t", " ", "");
	      if (iter->comments.length())
		{
		  fprintf(out, "\n");
		  print_string_list(out, iter->comments, "", "\n", "");
		}
	    }
	}
    }
      
  // global stuff.
  if (print_users)
    {
      fprintf(out, "\nUsers allowed to make deltas --\n");
      print_string_list(out, users, "\t", "\n", "everyone");
    }
  if (print_flags)
    {
      int flag_count = 0;
      fprintf(out, "\nFlags --\n");
      if (flags.branch)
	{
	  fprintf(out, "\tbranch\n");
	  ++flag_count;
	}
      
      print_flag(out, "\tceiling\t", flags.ceiling, flag_count);
      print_flag(out, "\tdefault SID\t", flags.default_sid, flag_count);
      print_flag(out, "\tfloor\t", flags.floor, flag_count);
      if (flags.no_id_keywords_is_fatal)
	{
	  fprintf(out, "\tid_keywd err/warn\n");
	  ++flag_count;
	}
      if (flags.joint_edit)
	{
	  fprintf(out, "\tjoint edit\n");
	  ++flag_count;
	}
      if (flags.all_locked)
	{
	  fprintf(out, "\tlocked releases a\n");
	}
      else if (!flags.locked.empty())
	{
	  fprintf(out, "\tlocked releases ");
	  flags.locked.print(out);
	  putc('\n', out);
	}
      print_flag(out, "\tmodule\t%s\n", flags.module, flag_count);
      print_flag(out, "\tnull delta\n", flags.null_deltas, flag_count);
      print_flag(out, "\tcsect name\t%s\n", flags.user_def, flag_count);
      print_flag(out, "\ttype\t%s\n", flags.type, flag_count);
      print_flag(out, "\tvalidate MRs\t%s\n", flags.mr_checker, flag_count);
    }
  if (print_desc)
    {
      fprintf(out, "\nDescription --\n");
      print_string_list(out, comments, "\t", "\n", "none");
    }

  if (print_body)
    {
      // seek_to_body() is a non-const member, so we have this
      // silly workaround.
      do_print_body((const char *)name, f, body_offset, out);
    }
}


/* Local variables: */
/* mode: c++ */
/* End: */