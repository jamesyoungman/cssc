/*
 * delta.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998,1999,2001 Free Software Foundation, Inc. 
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
 * Adds new deltas to SCCS files.
 *
 */

#include "cssc.h"
#include "my-getopt.h"
#include "fileiter.h"
#include "pfile.h"
#include "sccsfile.h"
#include "sf-chkmr.h"
#include "version.h"
#include "delta.h"
#include "except.h"



const char main_rcs_id[] = "CSSC $Id: delta.cc,v 1.31 2002/03/24 19:45:34 james_youngman Exp $";

void
usage() {
	fprintf(stderr,
"usage: %s [-nsV] [-m MRs] [-r SID] [-y comments] file ...\n",
		prg_name);
}

int
delta_main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int c;
  sid rid = NULL;		/* -r  ... XXX: does this need correcting? s/NULL/0/? */
  int silent = 0;		/* -s */
  int keep_gfile = 0;		/* -n */
#if 0
  sid_list ignore;		/* -g */
#endif
  mystring mrs;			/* -m -M */
  mystring comments;		/* -y -Y */
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

  class CSSC_Options opts(argc, argv, "r!sng!m!y!pV");
  for(c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next()) {
    switch (c) {
    default:
      errormsg("Unsupported option: '%c'", c);
      return 2;
			
    case 'r':
      rid = sid(opts.getarg());
      if (!rid.valid()) {
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

    case 'p':
      display_diff_output = true;
      break;

    case 'm':
      mrs = opts.getarg();
      suppress_mrs = (mrs == "");
      got_mrs = 1;
      break;
#if 0
      /* not a standard option */
    case 'M':
      mrs = "";
      suppress_mrs = 1;
      got_mrs = 1;
      break;
#endif
    case 'y':
      comments = opts.getarg();
      suppress_comments = (comments == "");
      got_comments = 1;
      break;

#if 0
      /* not a standard option */
    case 'Y':
      comments = "";
      suppress_comments = 1;
      got_comments = 1;
      break;
#endif

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

  mylist<mystring> mr_list, comment_list;
  int first = 1;

  int retval = 0;

  while (iter.next())
    {
#ifdef HAVE_EXCEPTIONS
      try
	{
#endif	    
	  bool failed = false;
	  sccs_name &name = iter.get_name();
	  sccs_file file(name, sccs_file::UPDATE);
	  sccs_pfile pfile(name, sccs_pfile::UPDATE);
		
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
		
	  switch (pfile.find_sid(rid)) {
	  case sccs_pfile::FOUND:
	    break;
		  
	  case sccs_pfile::NOT_FOUND:
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
	    retval = 1;
	    break;
		  
	  case sccs_pfile::AMBIGUOUS:
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
	    retval = 1;
	    break;
		  
	  default:
	    abort();
	  }
		
	  if (!failed)
	    {
	      if (!suppress_mrs && file.mr_required())
		{
		  if (mr_list.length() == 0)
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
		      pfile->delta.print(stdout);
		      putchar('\n');
		      errormsg("%s: Invalid MR number(s).",
			       name.c_str());
		      retval = 1;
		      continue;
		    }
		}
	      else if (mr_list.length())
		{
		  // MRs were specified and the MR flag is turned off.
		  pfile->delta.print(stdout);
		  putchar('\n');
		  errormsg("%s: MR verification ('v') flag not set, MRs"
			   " are not allowed.\n",
			   name.c_str());
		  retval = 1;
		  continue;
		}
		    
	      mystring gname = name.gfile();
		    
	      if (!file.add_delta(gname, pfile, mr_list, comment_list,
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
#ifdef HAVE_EXCEPTIONS
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
#endif	
    }
  return retval;
}


int
main(int argc, char **argv)
{
  int ret;
  ret = delta_main(argc, argv);
  return ret;
}


// Explicit template instantiations.
template class range_list<sid>;
template class mylist<mystring>;
template class mylist<delta>;
template class mylist<seq_no>;
template class mylist<sccs_pfile::edit_lock>;
template class mylist<char const*>;
//template mylist<char const*>& operator+=(mylist<char const *> &, mylist<mystring> const &);
template class range_list<release>;

#include "stack.h"
template class stack<unsigned short>;

/* Local variables: */
/* mode: c++ */
/* End: */

