/*
 * delta.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2007, 2008, 2009, 2010, 2011,
 *  2014, 2019 Free Software Foundation, Inc.
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
 * Adds new deltas to SCCS files.
 *
 */


#include <config.h>
#include <string>

#include "cssc.h"
#include "delta.h"
#include "cleanup.h"
#include "my-getopt.h"
#include "pfile.h"
#include "sccsfile.h"
#include "version.h"
#include "except.h"
#include "ioerr.h"
#include "file.h"
#include "fileiter.h"
#include "cssc.h"




void
usage() {
	fprintf(stderr,
"usage: %s [-nsVp] [-m MRs] [-r SID] [-y comments] file ...\n",
		prg_name);
}

#define EXITVAL_INVALID_OPTION (1)

static int
delta_main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int c;
  sid rid(sid::null_sid());
  int silent = 0;		/* -s */
  int keep_gfile = 0;		/* -n */
  std::string mrs;		/* -m -M */
  std::string comments;		/* -y -Y */
  int suppress_mrs = 0;		// if -m given with no arg.
  int got_mrs = 0;		// if no need to prompt for MRs.
  int suppress_comments = 0;	// if -y given with no arg.
  int got_comments = 0;
  bool display_diff_output = false; // -p
  if (argc > 0) {
    set_prg_name(argv[0]);
  } else {
    set_prg_name("delta");
  }

  ASSERT(!rid.valid());

  class CSSC_Options opts(argc, argv, "r!sng!m!y!pV", EXITVAL_INVALID_OPTION);
  for(c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next()) {
    switch (c) {
    default:
      errormsg("Unsupported option: '%c'", c);
      return EXITVAL_INVALID_OPTION;

    case 'r':
      rid = sid(opts.getarg());
      if (!rid.valid()) {
	errormsg("Invaild SID: '%s'", opts.getarg());
	return EXITVAL_INVALID_OPTION;
      }
      break;

    case 's':
      silent = 1;
      break;

    case 'n':
      keep_gfile = 1;
      break;

    case 'p':
      display_diff_output = true;
      break;

    case 'm':
      mrs = opts.getarg();
      suppress_mrs = (mrs == "");
      got_mrs = 1;
      break;

    case 'y':
      comments = opts.getarg();
      suppress_comments = (comments == "");
      got_comments = 1;
      break;

    case 'V':
      version();
      break;
    }
  }

  sccs_file_iterator iter(opts);
  if (iter.empty())
    {
      errormsg("No SCCS file specified.");
      return EXITVAL_INVALID_OPTION;
    }

  if (silent)
    {
      if (!stdout_to_null())
	{
	  return 1;		// fatal error.  Process no files.
	}
    }

  std::vector<std::string> comment_list;
  std::vector<std::string> mr_list;
  int first = 1;

  int retval = 0;

  while (iter.next())
    {
      try
	{
	  bool failed = false;
	  sccs_name &name = iter.get_name();
	  sccs_file file(name, UPDATE);
	  sccs_pfile pfile(name, sccs_pfile::pfile_mode::PFILE_UPDATE);

	  if (first)
	    {
	      if (!suppress_mrs && !got_mrs && file.mr_required())
		{
		  mrs = prompt_user("MRs? ");
		  got_mrs = 1;
		}
	      if (!suppress_comments && !got_comments)
		{
		  comments = prompt_user("comments? ");
		  got_comments = 1;
		}
	      mr_list = split_mrs(mrs);
	      comment_list = split_comments(comments);
	      first = 0;
	    }

	  const std::pair<sccs_pfile::find_status, sccs_pfile::iterator> found(pfile.find_sid(rid));
	  switch (found.first) {
	  case sccs_pfile::find_status::FOUND:
	    break;

	  case sccs_pfile::find_status::NOT_FOUND:
	    if (!rid.valid())
	      {
		errormsg("%s: You have no edits outstanding.",
			 name.c_str());
	      }
	    else
	      {
		errormsg("%s: Specified SID hasn't been locked for"
			 " editing by you.",
			 name.c_str());
	      }
	    failed = true;
	    retval = EXITVAL_INVALID_OPTION;
	    break;

	  case sccs_pfile::find_status::AMBIGUOUS:
	    if (rid.valid())
	      {
		errormsg("%s: Specified SID is ambiguous.",
			 name.c_str());
	      }
	    else
	      {
		errormsg("%s: You must specify a SID on the"
			 " command line.", name.c_str());
	      }
	    failed = true;
	    retval = EXITVAL_INVALID_OPTION;
	    break;

	  default:
	    abort();
	  }

	  if (!failed)
	    {
	      if (!suppress_mrs && file.mr_required())
		{
		  if (mr_list.empty())
		    {
		      errormsg("%s: MR number(s) must be supplied.",
			       name.c_str());
		      retval = 1;
		      continue;
		    }
		  if (file.check_mrs(mr_list))
		    {
		      /* In this case, _real_ SCCS prints the ID anyway.
		       */
		      found.second->delta.print(stdout);
		      putchar('\n');
		      errormsg("%s: Invalid MR number(s).",
			       name.c_str());
		      retval = 1;
		      continue;
		    }
		}
	      else if (!mr_list.empty())
		{
		  // MRs were specified and the MR flag is turned off.
		  found.second->delta.print(stdout);
		  putchar('\n');
		  errormsg("%s: MR verification ('v') flag not set, MRs"
			   " are not allowed.\n",
			   name.c_str());
		  retval = 1;
		  continue;
		}

	      std::string gname = name.gfile();

	      if (!file.add_delta(gname, pfile, found.second,
				  mr_list, comment_list,
				  display_diff_output))
		{
		  retval = 1;
		  // if delta failed, don't delete the g-file.
		}
	      else
		{
		  if (!keep_gfile)
		    {
		      /* SourceForge bug 489005: remove the g-file
		       * as the real user if we are running setuid.
		       */
		      if (!unlink_file_as_real_user(gname.c_str()))
			{
			  errormsg_with_errno("Failed to remove file %s",
					      gname.c_str());
			  retval = 1;
			}
		    }
		}
	    }
	}
      catch (CsscReallyFatalException e)
	{
	  exit(e.exitval);
	}
      catch (CsscExitvalException e)
	{
	  if (e.exitval > retval)
	    retval = e.exitval;	// continue with next file.
	}
    }
  return retval;
}


int
main(int argc, char **argv)
{
  return delta_main(argc, argv);
}

/* Local variables: */
/* mode: c++ */
/* End: */
