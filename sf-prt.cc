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
static const char rcs_id[] = "CSSC $Id: sf-prt.cc,v 1.1 1997/05/31 10:16:00 james Exp $";
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
  if (strlen(flag))
    {
      ++count;
      fprintf(out, fmt, (const char*)flag);
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

void		
sccs_file::prt(FILE *out,
	       sid cutoff_sid,	      // -y
	       sccs_date cutoff_date, // -c or -r
	       enum when when,	      // distinguished -c & -r
	       int all_deltas,	      // -a
	       int print_body,	      // -b
	       int print_delta_table, // -d
	       int print_flags,	      // -f
	       int incl_excl_ignore,  // -i
	       int first_line_only,   // -s
	       int print_desc,	      // -t
	       int print_users) const // -u

{
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
}


/* Local variables: */
/* mode: c++ */
/* End: */
