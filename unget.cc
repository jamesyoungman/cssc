/*
 * unget.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Removes edit locks from SCCS files.
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "pfile.h"
#include "my-getopt.h"
#include "version.h"
#include "except.h"


const char main_rcs_id[] = "CSSC $Id: unget.cc,v 1.22 2001/09/16 10:10:11 james_youngman Exp $";

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
  sid rid = NULL;		// (XXX: correct use of NULL?) 
  int silent = 0;
  int keep_gfile = 0;
  
  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("unget");

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
  if (! iter.using_source())
    {
      errormsg("No SCCS file specified.");
      return 1;
    }
  
  if (silent)
    {
      if (!stdout_to_null())
	{
	  return 1;		// fatal error.  Process no files.
	}
    }

  int retval = 0;
  const char *you = get_user_name();
  
  while (iter.next())
    {
#ifdef HAVE_EXCEPTIONS
      try
	{
#endif	  
	  sccs_name &name = iter.get_name();
	  sccs_pfile pfile(name, sccs_pfile::UPDATE);
	  
	  switch (pfile.find_sid(rid))
	    {
	      // normal case...
	    case sccs_pfile::FOUND:
	      if (!iter.unique())
		printf("\n%s:\n", name.c_str());
	      
	      pfile.print_lock_sid(stdout);
	      fputc('\n', stdout);
	      
	      pfile.delete_lock();
	      if (!pfile.update(true))
		retval = 1;
	      
	      if (!keep_gfile)
		{
		  mystring gname = name.gfile();
		  remove(gname.c_str());
		}
	      break;
	      
	      // error cases...
	    case sccs_pfile::NOT_FOUND:
	      if (!rid.valid())
		errormsg("%s: You have no edits outstanding.", name.c_str());
	      else
		errormsg("%s: Specified SID hasn't been locked for"
			 " editing by you (%s).",
			 name.c_str(), you);
	      retval = 1;
	      break;
	      
	    case sccs_pfile::AMBIGUOUS:
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
#ifdef HAVE_EXCEPTIONS
	}
      catch (CsscExitvalException e)
	{
	  if (e.exitval > retval)
	    retval = e.exitval;
	}
#endif
    }
  return retval;
}


// Explicit template instantiations.
template class range_list<sid>;
template class mylist<sccs_pfile::edit_lock>;
template class mylist<mystring>;



/* Local variables: */
/* mode: c++ */
/* End: */
