/*
 * prs.cc: Part of GNU CSSC.
 *
 *    Copyright (C) 1997,1998,1999,2001,2007,2008 Free Software Foundation, Inc.
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
 * Prints selected parts of an SCCS file.
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
usage() {
	fprintf(stderr,
"usage: %s [-aelDRV] [-c cutoff] [-d format] [-r SID] file ...\n",
		prg_name);
}

int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int c;
  mystring format = ":Dt:\t:DL:\nMRs:\n:MR:COMMENTS:\n:C:";
  sid rid(sid::null_sid());
  /* enum */ sccs_file::when selected = sccs_file::SIDONLY;
  bool all_deltas = false;
  sccs_date cutoff_date;
  int default_processing = 1;

  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("prs");

  ASSERT(!rid.valid());

  CSSC_Options opts(argc, argv, "d!Dr!elc!aV");
  for(c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next())
    {
      switch (c)
	{
	default:
	  errormsg("Unsupported option: '%c'", c);
	  return 2;

	case 'd':
	  format = opts.getarg();
	  /* specifying -d means, stop after the first match. */
	  default_processing = 0;
	  break;

	case 'D':	// obsolete MySC-ism.
	  default_processing = 0;
	  break;

	case 'r':
	  if (strlen(opts.getarg()))
	    {
	      rid = sid(opts.getarg());
	      if (!rid.valid() || rid.partial_sid())
		{
		  errormsg("Invaild SID: '%s'", opts.getarg());
		  return 2;
		}
	    }
	  else
	    {
	      // default: get the most recent delta.
	    }
	  default_processing = 0;
	  break;

	case 'c':
	  cutoff_date = sccs_date(opts.getarg());
	  if (!cutoff_date.valid())
	    {
	      errormsg("Invalid cutoff date: '%s'",
		       opts.getarg());
	      return 2;
	    }
	  break;

	case 'e':
	  selected = sccs_file::EARLIER;
	  default_processing = 0;
	  break;

	case 'l':
	  selected = sccs_file::LATER;
	  default_processing = 0;
	  break;

	case 'a':
	  all_deltas = true;
	  break;

	case 'V':
	  version();
	  break;
	}

    }

  if (selected == sccs_file::SIDONLY && cutoff_date.valid())
    {
      errormsg("Either the -e or -l switch must used with a"
	       " cutoff date.");
      return 2;
    }

  if (default_processing)
    {
      selected = sccs_file::EARLIER;
    }

  sccs_file_iterator iter(opts);
  if (! iter.using_source())
    {
      errormsg("No SCCS file specified.");
      return 1;
    }

  int retval = 0;

  while (iter.next())
    {
      try
	{
	  sccs_name &name = iter.get_name();
	  sccs_file file(name, sccs_file::READ);

	  if (default_processing)
	    {
	      printf("%s:\n\n", name.c_str());
	    }
	  bool matched = false;
	  if (!file.prs(stdout, format, rid, cutoff_date, selected,
			all_deltas, &matched))
	    {
	      retval = 1;
	    }
	  if (ferror(stdout))
	    {
	      errormsg("%s: Ouput file error.", name.c_str());
	      retval = 1;
	    }
	  if (!matched)
	    {
	      if (rid.valid()) 
		{
		  errormsg("%s: Requested SID doesn't exist.", name.c_str());
		  retval = 1;
		}
	    }
	} // end of try block.
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
