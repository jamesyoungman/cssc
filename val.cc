/*
 * val.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 *
 * Extract a requested delta from a SCCS file.
 * 
 */

#include "cssc.h"
#include "fileiter.h"
#include "sccsfile.h"
#include "my-getopt.h"
#include "version.h"
#include "except.h"
#include "valcodes.h"

const char main_rcs_id[] = "$Id: val.cc,v 1.4 2001/07/10 21:54:54 james_youngman Exp $";

/* Prints a list of included or excluded SIDs. */

void
usage()
{
  fprintf(stderr,
	  "usage: %s [-sV] [-m module] [-rSID] [-y type]\n",
	  prg_name);
}


int
main(int argc, char **argv)
{
  Cleaner arbitrary_name;
  int retval = 0;
  bool silent = false;
  mystring mstring;
  bool had_m_option = false;
  mystring ystring;
  bool had_y_option = false;
  bool had_r_option = false;
  int c;
  const char *req_sid_str = NULL;
  sid rid = NULL;		// (XXX: correct use of NULL?) 
  
  if (argc > 0)
      set_prg_name(argv[0]);
  else
    set_prg_name("val");


  class CSSC_Options opts(argc, argv, "sV!m!r!y!");
  for(c = opts.next();
      c != CSSC_Options::END_OF_ARGUMENTS;
      c = opts.next())
    {
      switch (c)
	{
	default:
	  errormsg("Unsupported option: '%c'", c);
	  retval |= Val_InvalidOption;
	  return retval;
	  
	case 'r':
	  if (had_r_option)
	    {
	      errormsg("Duplicate -r option\n");
	      retval |= Val_InvalidOption;
	    }
	  had_r_option = true;
	  req_sid_str = opts.getarg();
	  rid = sid(req_sid_str);
	  if (!rid.valid())
	    {
	      errormsg("Invaild SID: '%s'", opts.getarg());
	      retval |= Val_InvalidSID;
	    }
	  break;
	  
	case 's':
	  silent = true;
	  break;

	case 'm':
	  if (had_m_option)
	    {
	      errormsg("Duplicate -m option\n");
	      retval |= Val_InvalidOption;
	    }
	  had_m_option = true;
	  mstring = mystring(opts.getarg());
	  break;

	case 'y':
	  if (had_y_option)
	    {
	      errormsg("Duplicate -y option\n");
	      retval |= Val_InvalidOption;
	    }
	  had_y_option = true;
	  ystring = mystring(opts.getarg());
	  break;

	case 'V':
	  version();
	  break;
	}
    }

  if (silent)
    {
      if (!stdout_to_null())
	return 1;	// fatal error.
    }
  
  sccs_file_iterator iter(opts);
  if (sccs_file_iterator::NONE == iter.using_source())
    {
      errormsg("No SCCS file specified");
      retval |= Val_MissingFile;
      return retval;
    }
  
  while (iter.next())
    {
#ifdef HAVE_EXCEPTIONS
      try
	{
#endif	      
	  sccs_name &name = iter.get_name();
	  
	  
	  sccs_file file(name, sccs_file::READ);
	      
	  if (had_r_option)
	    {
	      if (NULL == file.find_delta(rid))
		{
		  if (!silent)
		    {
		      errormsg("%s: Requested SID %s not found.",
			       name.c_str(), req_sid_str);
		    }
		  retval |= Val_NoSuchSID;
		}
	    }
	  
	  if (had_m_option)
	    {
	      const mystring &module_flag = file.get_module_name();
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
		  retval |= Val_MismatchedM;
		}
	    }

	  if (had_y_option)
	    {
	      const mystring &type_flag = file.get_module_type_flag();
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
		  retval |= Val_MismatchedY;
		}
	    }
	  
	  
	  if (!file.validate())
	    {
	      retval |= Val_CorruptFile;
	    }
	  
	  

#ifdef HAVE_EXCEPTIONS
	}
      
      catch (CsscSfileCorruptException ce)
	{
	  retval |= Val_CorruptFile;
	}
      
      catch (CsscExitvalException e)
	{
	  if (e.exitval > retval)
	    retval = e.exitval;
	}

      catch (CsscContstructorFailedException e)
	{
	  retval |= Val_CorruptFile;
	}
#endif		
    }

  return retval;
}



/* Local variables: */
/* mode: c++ */
/* End: */
