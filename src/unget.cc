/*
 * unget.cc: Part of GNU CSSC.
 *
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
 * Removes edit locks from SCCS files.
 *
 */

#include <config.h>
#include <string>
#include "cssc.h"
#include "fileiter.h"
#include "pfile.h"
#include "my-getopt.h"
#include "version.h"
#include "except.h"
#include "file.h"


void
usage() {
	fprintf(stderr,
"usage: %s [-nsV] [-r SID] file ...\n",
		prg_name);
}

int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int c;
  sid rid(sid::null_sid());
  int silent = 0;
  int keep_gfile = 0;

  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("unget");

  ASSERT(!rid.valid());

  class CSSC_Options opts(argc, argv, "r!snV");
  for (c = opts.next(); c != CSSC_Options::END_OF_ARGUMENTS; c = opts.next())
    {
      switch (c)
	{
	default:
	  errormsg("Unsupported option: '%c'", c);
	  return 2;

	case 'r':
	  rid = sid(opts.getarg());
	  if (!rid.valid())
	    {
	      errormsg("Invaild SID: '%s'", opts.getarg());
	      return 2;
	    }
	  break;

	case 's':
	  silent = 1;
	  break;

	case 'n':
	  keep_gfile = 1;
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
      return 1;
    }

  if (silent)
    {
      if (!stdout_to_null().ok())
	{
	  return 1;		// fatal error.  Process no files.
	}
    }

  int retval = 0;
  const char *you = get_user_name();

  while (iter.next())
    {
      try
	{
	  sccs_name &name = iter.get_name();
	  sccs_pfile pfile(name, sccs_pfile::pfile_mode::PFILE_UPDATE);

	  const std::pair<sccs_pfile::find_status, sccs_pfile::iterator> found(pfile.find_sid(rid));
	  switch (found.first)
	    {
	      // normal case...
	    case sccs_pfile::find_status::FOUND:
	      if (!iter.unique())
		printf("\n%s:\n", name.c_str());

	      pfile.print_lock_sid(stdout, found.second);
	      fputc('\n', stdout);

	      pfile.delete_lock(found.second);
	      if (!pfile.update(true))
		retval = 1;

	      if (!keep_gfile)
		{
		  std::string gname = name.gfile();
		  remove(gname.c_str());
		}
	      break;

	      // error cases...
	    case sccs_pfile::find_status::NOT_FOUND:
	      if (!rid.valid())
		errormsg("%s: You have no edits outstanding.", name.c_str());
	      else
		errormsg("%s: Specified SID hasn't been locked for"
			 " editing by you (%s).",
			 name.c_str(), you);
	      retval = 1;
	      break;

	    case sccs_pfile::find_status::AMBIGUOUS:
	      if (!rid.valid())
		errormsg("%s: Specified SID is ambiguous.", name.c_str());
	      else
		errormsg("%s: You must specify a SID on the"
			 " command line.", name.c_str());
	      retval = 1;
	      break;

	    default:
	      abort();
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
