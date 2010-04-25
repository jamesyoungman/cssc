/*
 * cdc.cc: Part of GNU CSSC.
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
 * Changes the comments and MRs of a delta.
 *
 */

#include "cssc.h"
#include "my-getopt.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "sf-chkmr.h"
#include "version.h"
#include "delta.h"
#include "except.h"
#include "file.h"


void
usage()
{
  fprintf(stderr,
	  "usage: %s [-V] [-m MRs] [-y comments] -r SID file ...\n",
	  prg_name);
}

static const char *
plural(int n)
{
  if (1 == n)
    return "";
  else
    return "s";
}

int
main(int argc, char *argv[])
{
  Cleaner arbitrary_name;
  int c;
  sid rid(sid::null_sid());
  mystring mrs;
  mystring comments;
  int got_comments = 0;
  int retval = 0;
  
  if (argc > 0)
    set_prg_name(argv[0]);
  else
    set_prg_name("cdc");

  ASSERT(!rid.valid());

  CSSC_Options opts(argc, argv, "r!m!y!V");
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
	  
	case 'm':
	  mrs = opts.getarg();
	  break;
	  
	case 'y':
	  comments = opts.getarg();
	  got_comments = 1;
	  break;
	  
	case 'V':
	  version();
	  break;
	}
    }
  
  if (!rid.valid())
    {
      errormsg("A SID must be specified on the command line.");
      return 1;
    }
  
  sccs_file_iterator iter(opts);
  if (! iter.using_source())
    {
      errormsg("No SCCS file specified.");
      return 1;
    }
  
  if (!got_comments)
    {
      if (!iter.using_stdin())
	{
	  comments = prompt_user("comments? ");
	}
      else
	{
	  /* No -y option specified, but "-" was
	   * specified on the command line, so
	   * that's an error.
	   */
	  errormsg("You can't use standard input for the argument list "
		   "without\n"
		   "using the \"-y\" option, because these two uses of "
		   "stdin are mutually\n"
		   "exclusive, sorry.");
	  return 1;
	}
    }
  
  mylist<mystring> mr_list, comment_list;
  comment_list = split_comments(comments);
  mr_list = split_mrs(mrs);
  
  int tossed_privileges = 0;
  
  int first = 1;
  
  while (iter.next())
    {
      try
	{
	  sccs_name &name = iter.get_name();
	  sccs_file file(name, sccs_file::UPDATE);

	  // If we need an MR list, prompt if required.
	  if (first)
	    {
	      first = 0;
	      if (file.mr_required() && (0 == mr_list.length()) )
		{
		  if (iter.using_stdin())
		    {
		      /* No -y option specified, but "-" was
		       * specified on the command line, so
		       * that's an error.
		       */
		      errormsg("You can't use standard input for argument list "
			       "without using the \"-y\" and \"-m\" options, "
			       "because\nthese two uses of stdin are mutually"
			       " exclusive, sorry.");
		      return 1;
		    }
		  else
		    {
		      mrs = prompt_user("MRs? ");
		      mr_list = split_mrs(mrs);
		    }
		}
	    }
  
	  if (mr_list.length() != 0)
	    {
	      if (file.mr_required())
		{
		  if (file.check_mrs(mr_list))
		    {
		      errormsg("%s: Invalid MR number%s -- validation failed..",
			       plural(mr_list.length()), name.c_str());
		      retval = 1;
		      continue;	// with next file.
		    }
		}
	      else 
		{
		  /* No MRs required; it is in this case an error to provide them! */
		  errormsg("%s: MRs are not allowed for this file "
			   "(because its 'v' flag is not set).",
			   name.c_str());
		  retval = 1;
		  continue;		// with next file...
		}
	    }
	  else
	    {
	      if (file.mr_required())
		{
		  errormsg("%s: MRs required.", name.c_str());
		  retval = 1;
		  continue;
		}
	  
	    }
      
      
	  if (!file.is_delta_creator(get_user_name(), rid))
	    {
	      give_up_privileges();
	      tossed_privileges = 1;
	    }
      
	  if (file.cdc(rid, mr_list, comment_list))
	    {
	      if (!file.update())
		retval = 1;
	    }
	  else
	    {
	      retval = 1;
	    }

	  if (tossed_privileges)
	    {
	      restore_privileges();
	      tossed_privileges = 0;
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
