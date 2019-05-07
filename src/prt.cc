/*
 * prt.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014, 2019
 *  Free Software Foundation, Inc.
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
 * Print delta table information from an SCCS file.
 *
 */


#include <config.h>
#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "my-getopt.h"
#include "version.h"
#include "delta.h"
#include "except.h"


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
  Cleaner arbitrary_name;
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

  class CSSC_Options opts(argc, argv, "abdefistuVc!r!y!");
  for(int c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next())
    {
      switch (c)
	{
	default:
	  errormsg("Unsupported option: '%c'", c);
	  return 2;

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
		  errormsg("Invaild SID: '%s'", opts.getarg());
		  return 2;
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
	    {
	      errormsg("Options -c and -r are exclusive.\n");
	      return 2;
	    }

	  last_cutoff_type = (int)c;

	  sccs_date date = sccs_date(opts.getarg());
	  if (!date.valid())
	    {
	      errormsg("Invalid cutoff date: '%s'", opts.getarg());
	      return 2;
	    }


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

  int retval = 0;

  if (sccs_file_iterator::NONE == iter.using_source())
    {
      errormsg("No SCCS file specified");
      return 1;
    }

  while (iter.next())
    {
      try
	{
	  sccs_name &name = iter.get_name();

	  if (!exclude.enabled)
	    fprintf(stdout, "\n%s:", name.c_str());

	  fprintf(stdout, "\n");

	  sccs_file file(name, sccs_file::READ);

	  if (!file.prt(stdout,
			exclude,		  // -y, -c, -r
			all_deltas,		  // -a
			print_body,		  // -b
			print_delta_table,	  // -d
			print_flags,	  // -f
			incl_excl_ignore,	  // -i
			first_line_only,	  // -s
			print_desc,		  // -t
			print_users))	  // -u
	    {
	      retval = 1;
	    }
	}
      catch (CsscExitvalException e)
	{
	  if (e.exitval > retval)
	    retval = e.exitval;
	}
    }
  return retval;
}


/* Local variables: */
/* mode: c++ */
/* End: */
