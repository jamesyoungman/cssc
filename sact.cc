/*
 * sact.cc: Part of GNU CSSC.
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Prints the current edit locks for an SCCS file. 
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "pfile.h"
#include "version.h"
#include "my-getopt.h"
#include "except.h"

const char main_rcs_id[] = "CSSC $Id: sact.cc,v 1.16 2003/12/08 12:13:46 james_youngman Exp $";

void
usage() {
	fprintf(stderr,
"usage: %s [-V] file ...\n",
		prg_name);
}

int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("sact");


  class CSSC_Options opts(argc, argv, "V");
  int c;
  for (c = opts.next(); c != CSSC_Options::END_OF_ARGUMENTS; c = opts.next())
    {
      switch (c)
	{
	case 'V':
	  version();
	  break;
	}
    }

  int retval = 0;
  sccs_file_iterator iter(opts);
  if (! iter.using_source())
    {
      errormsg("No SCCS file specified.");
      return 1;
    }
  
  while (iter.next())
    {
#ifdef HAVE_EXCEPTIONS
      try
	{
#endif	  
	  sccs_name &name = iter.get_name();
	  sccs_pfile pfile(name, sccs_pfile::READ);
	  
	  pfile.rewind();
	  
	  bool first = true;
	  while (pfile.next())
	    {
	      if (first) // first lock on this file...
		{
		  /*
		   * Before we print out the information about the
		   * first lock on a given file. we may have to
		   * identify which file we are talking about.  We
		   * don't do this if only one file was specified on
		   * the command line.
		   */
		  if (!iter.unique())
		  {
		    printf("\n%s:\n", name.c_str());
		  }
		  first = false;
		}
	      
	      pfile->got.print(stdout);
	      putchar(' ');
	      pfile->delta.print(stdout);
	      putchar(' ');
	      fputs(pfile->user.c_str(), stdout);
	      putchar(' ');
	      pfile->date.print(stdout);
	      putchar('\n');
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
