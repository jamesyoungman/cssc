/*
 * rmdel.cc: Part of GNU CSSC.
 *
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
 * Removes a delta from an SCCS file.
 *
 */

#include <config.h>
#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "pfile.h"
#include "my-getopt.h"
#include "version.h"
#include "delta.h"
#include "except.h"
#include "file.h"


void
usage() {
	fprintf(stderr,
"usage: %s [-V] -r SID file ...\n",
		prg_name);
}


static bool
is_locked(sccs_name& name, sid rid)
{
  sccs_pfile pfile(name, sccs_pfile::READ);
  for (sccs_pfile::const_iterator it = pfile.begin();
       it != pfile.end();
       ++it)
    {
      if (it->got == rid)
	return true;
    }
  return false;
}


int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int c;
  sid rid(sid::null_sid());

  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("rmdel");

  ASSERT(!rid.valid());

  class CSSC_Options opts(argc, argv, "r!V");
  for (c = opts.next(); c != CSSC_Options::END_OF_ARGUMENTS;
       c = opts.next())
    {
      switch (c)
	{
	case 'r':
	  rid = sid(opts.getarg());
	  if (!rid.valid())
	    {
	      errormsg("Invaild SID: '%s'", opts.getarg());
	      return 2;
	    }
	  break;

	case 'V':
	  version();
	  break;
	}
    }

  if (!rid.valid())
    {
      errormsg("A SID must be specified on the command line.");
      return 2;
    }

  sccs_file_iterator iter(opts);
  if (! iter.using_source())
    {
      errormsg("No SCCS file specified.");
      return 1;
    }

  int tossed_privileges = 0;
  int retval = 0;

  while (iter.next())
    {
      try
	{
	  sccs_name &name = iter.get_name();
	  sccs_file file(name, sccs_file::UPDATE);

	  if (is_locked(name, rid))
	    {
	      errormsg("%s: Requested SID is locked for editing.",
		       name.c_str());
	      retval = 1;
	    }
	  else
	    {
	      if (!file.is_delta_creator(get_user_name(), rid))
		{
		  give_up_privileges();
		  tossed_privileges = 1;
		}

	      if (!file.rmdel(rid))
		retval = 1;

	      if (tossed_privileges)
		{
		  restore_privileges();
		  tossed_privileges = 0;
		}
	    }
	}
      catch (CsscExitvalException e)
	{
	  if (tossed_privileges)
	    {
	      restore_privileges();
	      tossed_privileges = 0;
	    }
	  if (e.exitval > retval)
	    retval = e.exitval;
	}
    }
  return retval;
}

/* Local variables: */
/* mode: c++ */
/* End: */
