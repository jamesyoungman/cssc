/*
 * val.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 2001, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
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
 *
 * Validate a SCCS file.
 *
 */

#include <config.h>

#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "my-getopt.h"
#include "version.h"
#include "except.h"
#include "file.h"
#include "valcodes.h"

/* Prints a list of included or excluded SIDs. */

void
usage()
{
  fprintf(stderr,
	  "usage: %s [-sV] [-m module] [-rSID] [-y type]\n",
	  prg_name);
}


static void problem(int &retval, const int &newprob)
{
  if (newprob > retval)
    retval = newprob;
}


int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int retval = 0;
  bool silent = false;
  std::string mstring;
  bool had_m_option = false;
  std::string ystring;
  bool had_y_option = false;
  bool had_r_option = false;
  int c;
  const char *req_sid_str = NULL;
  sid rid(sid::null_sid());

  if (argc > 0)
      set_prg_name(argv[0]);
  else
    set_prg_name("val");

  ASSERT(!rid.valid());

  class CSSC_Options opts(argc, argv, "sV!m!r!y!", 0);
  for(c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next())
    {
      switch (c)
	{
	case CSSC_Options::UNRECOGNIZED_OPTION:
	case CSSC_Options::MISSING_ARGUMENT:
	  problem(retval, Val_InvalidOption);
	  return retval;

	default:
	  errormsg("Unsupported option: '%c'", c);
	  problem(retval, Val_InvalidOption);
	  return retval;

	case 'r':
	  if (had_r_option)
	    {
	      errormsg("Duplicate -r option\n");
	      problem(retval, Val_InvalidOption);
	    }
	  had_r_option = true;
	  req_sid_str = opts.getarg();
	  rid = sid(req_sid_str);
	  if (!rid.valid())
	    {
	      errormsg("Invaild SID: '%s'", opts.getarg());
	      problem(retval, Val_InvalidSID);
	    }
	  break;

	case 's':
	  silent = true;
	  break;

	case 'm':
	  if (had_m_option)
	    {
	      errormsg("Duplicate -m option\n");
	      problem(retval, Val_InvalidOption);
	    }
	  had_m_option = true;
	  mstring = std::string(opts.getarg());
	  break;

	case 'y':
	  if (had_y_option)
	    {
	      errormsg("Duplicate -y option\n");
	      problem(retval, Val_InvalidOption);
	    }
	  had_y_option = true;
	  ystring = std::string(opts.getarg());
	  break;

	case 'V':
	  version();
	  break;
	}
    }

  if (silent)
    {
      if (!stdout_to_null().ok())
	return 1;	// fatal error.
    }

  sccs_file_iterator iter(opts);
  if (iter.empty())
    {
      errormsg("No SCCS file specified");
      problem(retval, Val_MissingFile);
      return retval;
    }

  while (iter.next())
    {
      try
	{
	  sccs_name &name = iter.get_name();
	  sccs_file file(name, READ);

	  if (had_r_option)
	    {
	      if (rid.valid() && NULL == file.find_delta(rid))
		{
		  if (!silent)
		    {
		      errormsg("%s: Requested SID %s not found.",
			       name.c_str(), req_sid_str);
		    }
		  problem(retval, Val_NoSuchSID);
		}
	    }

	  if (had_m_option)
	    {
	      const std::string &module_flag = file.get_module_name();
	      if (module_flag != mstring)
		{
		  if (!silent)
		    {
		      errormsg("%s: mismatch for %%"
			       "M%%: wanted \"%s\", got \"%s\"\n",
			       name.c_str(),
			       mstring.c_str(),
			       module_flag.c_str());
		    }
		  problem(retval, Val_MismatchedM);
		}
	    }

	  if (had_y_option)
	    {
	      const std::string &type_flag = file.get_module_type_flag();
	      if (type_flag != ystring)
		{
		  if (!silent)
		    {
		      errormsg("%s: mismatch for %%"
			       "Y%%: wanted \"%s\", got \"%s\"\n",
			       name.c_str(),
			       mstring.c_str(),
			       type_flag.c_str());
		    }
		  problem(retval, Val_MismatchedY);
		}
	    }


	  if (!file.validate())
	    {
	      problem(retval, Val_CorruptFile);
	    }
	}
      catch (CsscSfileMissingException e)
	{
	  problem(retval, Val_CannotOpenOrWrongFormat);
	}
      catch (CsscContstructorFailedException e)
	{
	  problem(retval, Val_CorruptFile);
	}
      catch (CsscSfileCorruptException ce)
	{
	  problem(retval, Val_CorruptFile);
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
