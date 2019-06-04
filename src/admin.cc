/*
 * admin.cc: Part of GNU CSSC.
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
 * Administer and create SCCS files.
 *
 */
#include <config.h>

#include <unordered_set>
#include <errno.h>

#include "cssc.h"
#include "sccsfile.h"
#include "fileiter.h"
#include "sid_list.h"
#include "sl-merge.h"
#include "my-getopt.h"
#include "version.h"
#include "delta.h"
#include "except.h"


static bool
well_formed_sccsname(const sccs_name& n)
{
  const char *s;

  if ( nullptr != (s = strrchr(n.c_str(), '/')) )
    {
      s++;
    }
  else
    {
      s = n.c_str();
    }
  return ('s' == s[0]) && ('.' == s[1]);
}

void
usage() {
	fprintf(stderr,
"usage: %s [-nrzV] [-a users] [-d flags] [-e users] [-f flags]\n"
"\t[-i file] [-m MRs] [-t file] [-y comments] file ...\n",
	prg_name);
}

int
main(int argc, char **argv)
{
  int c;
  Cleaner arbitrary_name;
  sid first_sid("1.1");		                /* -r */
  bool got_r_option = false;
  int new_sccs_file = 0;			/* -n */
  bool force_binary = false;			/* -b */
  const char *iname = NULL;			/* -i -I */
  const char *file_comment = NULL;		/* -t */
  std::vector<std::string> set_flags, unset_flags;	/* -f, -d */
  std::vector<std::string> add_users;			/* -a  */
  std::unordered_set<std::string> erase_users;	/* -e */
  std::string mrs, comments;				/* -m, -y */
  int check_checksum = 0;	                /* -h */
  int validate       = 0;	                /* also -h */
  int reset_checksum = 0;			/* -z */
  int suppress_mrs = 0;				/* -m " " (i.e. no actual MRs) */
  int suppress_comments = 0;			/* -y (no arg) */
  int empty_t_option = 0;	                /* -t (no arg) */
  int retval;


#ifdef __GLIBC__
# if __GNUC_PREREQ (2,1)
  // The bug mentioned below is now fixed.
# else
  // If we use the "-i" option, we read the initial body from the
  // named file, or stdin if no file is named.  In this case, we use
  // fgetpos() or ftell() so that we can rewind the input in order to
  // try again with encoding, should we find the body to need it.
  // GNU glibc version 2.0.6 has a bug which results in ftell()/fgetpos()
  // succeeding on stdin even if it is a pipe,fifo,tty or other
  // nonseekable object.  We get around this bug by setting stdin to
  // un-buffered, which avoids this bug.
  setvbuf(stdin, (char*)0, _IONBF, 0u);
# endif
#endif

  if (argc > 0) {
    set_prg_name(argv[0]);
  } else {
    set_prg_name("admin");
  }

  retval = 0;

  class CSSC_Options opts(argc, argv, "bni!r!t!f!d!a!e!m!y!hzV");
  for (c = opts.next();
       c != CSSC_Options::END_OF_ARGUMENTS;
       c = opts.next()) {
    switch (c) {
    default:
      errormsg("Unsupported option: '%c'", c);
      return 2;

    case 'n':
      new_sccs_file = 1;
      break;

    case 'b':
      if (binary_file_creation_allowed())
	{
	  force_binary = true;
	}
      else
	{
	  errormsg("Option -b specified but binary file support is disabled.");
	  return 2;
	}
      break;

    case 'i':
      if (strlen(opts.getarg()) > 0)
	iname = opts.getarg();
      else
	iname = "-";
      new_sccs_file = 1;
      break;

    case 'r':
      {
	got_r_option = true;

	const char *rel_str = opts.getarg();
	release r(rel_str);
	if (r.valid())
	  {
	    first_sid = sid(r).successor();
	  }
	else
	  {
	    // The user make have specified (e.g.) "1.1" or "1.2" or "1.1.1.1",
	    // which the SCCS manpages don't state is supported, but in fact
	    // some implementations do support this.

	    warning("some SCCS implementations allow only "
		     "a release number for the -r option.\n");

	    first_sid = sid(rel_str);
	    if (!first_sid.valid())
	      {
		errormsg("Invaild initial release: '%s'", rel_str);
		return 1;
	      }
	  }
	break;
      }

    case 't':
      file_comment = opts.getarg();
      empty_t_option = (0 == strlen(file_comment));
      break;

    case 'f':
      set_flags.push_back(opts.getarg());
      break;

    case 'd':
      unset_flags.push_back(opts.getarg());
      break;

    case 'a':
      add_users.push_back(opts.getarg());
      break;

    case 'e':
      erase_users.insert(opts.getarg());
      break;

    case 'm':
      mrs = opts.getarg();
      suppress_mrs = true;
      break;

    case 'y':
      comments = opts.getarg();
      suppress_comments = (comments == "");
      break;

    case 'h':
      check_checksum = 1;
      validate = 1;
      break;

    case 'z':
      reset_checksum = 1;
      break;

    case 'V':
      version();
      if (2 == argc)
	return 0; // "admin -V" should succeed.
    }
  }

  if (empty_t_option && new_sccs_file)
    {
      errormsg("The -t option must have an argument "
	       "if -n or -i is used.");
      return 1;
    }

  if (check_checksum)
      reset_checksum = false;

  std::vector<std::string> comment_list;
  std::vector<std::string> mr_list;
  int was_first = 1;
  sccs_file_iterator iter(opts);
  if (iter.empty())
    {
      errormsg("No SCCS file specified.");
      return 1;
    }


  if (got_r_option && !iname)
    {
      errormsg("You must use the -i option if you use the -r option.");
      return 1;
    }


  while (iter.next())
    {
      try
	{
	  // can't set first to 0 at end of loop because we may
	  // use "continue".
	  int me_first = was_first;
	  was_first = 0;

	  sccs_name &name = iter.get_name();


	  if (!well_formed_sccsname(name))
	    {
	      errormsg("%s is an invalid name."
		       "  SCCS file names must begin `s.'\n",
		       name.c_str());
	      retval = 1;
	      continue;	// with next file
	    }

	  sccs_file_open_mode mode = sccs_file_open_mode::READ;
	  if (check_checksum)
	    mode = sccs_file_open_mode::READ;
	  else if (new_sccs_file)
	    mode = sccs_file_open_mode::CREATE;
	  else if (reset_checksum)
	    mode = sccs_file_open_mode::FIX_CHECKSUM;
	  else
	    mode = sccs_file_open_mode::UPDATE;

	  sccs_file file(name, mode);

	  // The -h option (check_checksum and validate) overrides all the
	  // other options; if you specify the -h flag, no other action
	  // will take place.
	  if (check_checksum || validate)
	    {
	      if (check_checksum)
		{
		  if (!file.checksum_ok())
		    retval = 1;
		}

	      if (validate)
		{
		  // Validate the file itself.   The original sccs suite
		  // probably does this by running "val" from "admin", but
		  // we prefer not to exec things (for security reasons,
		  // among others).
		  if (!file.validate())
		    retval = 1; // Validation failed.
		}

	      continue;
	    }


	  if (reset_checksum)
	    {
	      if (!file.update_checksum())
		retval = 1; // some kind of error.

	      continue;
	    }


	  if (!file.admin(file_comment,
			  force_binary, set_flags, unset_flags,
			  add_users, erase_users))
	    {
	      retval = 1;
	      continue;
	    }


	  if (new_sccs_file)
	    {
	      if (iname != NULL && !me_first)
		{
		  errormsg("The 'i' keyletter can't be used with"
			   " multiple files.");
		  return 1;
		}

	      if (me_first)
		{
		  // The real thing does not prompt the user here.
		  // Hence we don't either.
		  if (!file.mr_required() && !mrs.empty())
		    {
		      errormsg("MRs not enabled with 'v' flag, "
			       "can't use 'm' keyword.");
		      retval = 1;
		      continue; // with next file
		    }

		  mr_list = split_mrs(mrs);
		  if (!mr_list.empty())
		    {
		      suppress_mrs = false;
		    }
		  comment_list = split_comments(comments);
		}

	      if (file.mr_required())
		{
		  if (!suppress_mrs && mr_list.empty())
		    {
		      errormsg("%s: MR number(s) must be supplied.", name.c_str());
		      retval = 1;
		      continue; // with next file

		    }
		  if (file.check_mrs(mr_list))
		    {
		      errormsg("%s: Invalid MR number(s).",
			       name.c_str());
		      retval = 1;
		      continue; // with next file
		    }
		}
	      errno = 0;
	      if (!file.create(first_sid, iname,
			       mr_list, &comment_list,
			       suppress_comments, force_binary))
		{
		  retval = 1;
		  continue;
		}

	    }
	  else
	    {
	      if (!file.update())
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
