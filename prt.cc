/*
 * prt.cc
 *
 * Print delta table information from an SCCS file.
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "my-getopt.h"
#include "version.h"
#include "delta.h"

const char main_rcs_id[] = "CSSC $Id: prt.cc,v 1.7 1998/02/21 14:03:57 james Exp $";

void
usage()
{
  fprintf(stderr,
	  "usage: %s %s", prg_name, 
	  "[-abdefistu] [-cDATE-TIME] [-rDATE-TIME] [-ySID] s.file ...\n");
}


/* 
 * The effects of the options on sccs-prt are quite complex; for
 * example, -y affects the output mode as well.
 */
int
main(int argc, char **argv)
{
  int all_deltas = 0;		// -a
  int print_body = 0;		// -b
  int print_delta_table = 0;	// -d
  int print_flags = 0;		// -f
  int incl_excl_ignore = 0;	// -i
  int first_line_only = 0;	// -f
  int print_desc = 0;		// -t
  int print_users = 0;		// -u
  sccs_file::cutoff exclude;
  int last_cutoff_type = 0;
  int do_default = 1;
  
  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("prt");

  class getopt opts(argc, argv, "abdefistuVc!r!y!");
  for(int c = opts.next(); c != getopt::END_OF_ARGUMENTS; c = opts.next())
    {
      switch (c)
	{
	default:
	  quit(-2, "Unsupported option: '%c'", c);

	case 'a':
	  all_deltas = 1;
	  break;
	case 'b':
	  print_body = 1;
	  do_default = 0;
	  break;
	case 'd':
	  print_delta_table = 1;
	  break;
	case 'e':		// implies -diuft
	  print_delta_table = incl_excl_ignore = 1;
	  print_users = print_flags = print_desc = 1;
	  break;
	case 'f':
	  print_flags = 1;
	  do_default = 0;
	  break;
	case 'i':		// -s and -i are exclusive.
	  incl_excl_ignore = 1;
	  first_line_only = 0;
	  break;
	case 's':		// -s and -i are exclusive.
	  first_line_only = 1;
	  incl_excl_ignore = 0;
	  break;
	case 't':
	  print_desc = 1;
	  do_default = 0;
	  break;
	case 'u':
	  print_users = 1;
	  do_default = 0;
	  break;
	case 'V':
	  version();
	  break;

	case 'y':
	  exclude.enabled = true;
	  if (strlen(opts.getarg()))
	    {
	      exclude.cutoff_sid = sid(opts.getarg());
	      exclude.most_recent_sid_only = false;
	      if (!exclude.cutoff_sid.valid()
		  || exclude.cutoff_sid.partial_sid())
		{
		  quit(-2, "Invaild SID: '%s'", opts.getarg());
		}
	    }
	  else			// empty -y arg.
	    {
	      exclude.most_recent_sid_only = true;
	    }
	  break;
	  
	case 'c':		// -c and -r
	case 'r':		// are exclusive.
	  exclude.enabled = true;
	  if (0 != last_cutoff_type && c != last_cutoff_type)
	    quit(-2, "Options -c and -r are exclusive.\n");
	  last_cutoff_type = (int)c;
	  
	  sccs_date date = sccs_date(opts.getarg());
	  if (!date.valid())
	    quit(-2, "Invalid cutoff date: '%s'", opts.getarg());
	  
	  if (c == 'r')
	    exclude.last_accepted = date;
	  else
	    exclude.first_accepted = date;
	  break;
	}

    }
  if (do_default)		// none of -uftb specified...
    print_delta_table = 1;	// ...so assume -d.

  sccs_file_iterator iter(opts);

  // exclude.print(stdout);
  
  while(iter.next())
    {
      sccs_name &name = iter.get_name();

      fprintf(stdout, "\n%s:", name.c_str());

      sccs_file file(name, sccs_file::READ);

      file.prt(stdout,
	       exclude,		  // -y, -c, -r
	       all_deltas,	  // -a
	       print_body,	  // -b
	       print_delta_table, // -d
	       print_flags,	  // -f
	       incl_excl_ignore,  // -i
	       first_line_only,	  // -s
	       print_desc,	  // -t
	       print_users);	  // -u
    }

  return 0;
}


// Explicit template instantiations.
template class list<mystring>;
template class list<seq_no>;
template class range_list<release>;
template class list<delta>;
// template class range_list<sid>;


// #include "stack.h"
// template class stack<seq_no>;

/* Local variables: */
/* mode: c++ */
/* End: */
