/*
 * sact.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998, Free Software Foundation, Inc. 
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
 * Prints the current edit locks for an SCCS file. 
 *
 */

#include "cssc.h"
#include "fileiter.h"
#include "pfile.h"
#include "version.h"
#include "my-getopt.h"
#include "except.h"

const char main_rcs_id[] = "CSSC $Id: sact.cc,v 1.11 1998/09/03 22:21:37 james Exp $";

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
	      if (first)		// first lock on this file...
		{
		  printf("\n%s:\n", name.c_str());
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
template class list<sccs_pfile::edit_lock>;
template class list<mystring>;

/* Local variables: */
/* mode: c++ */
/* End: */
