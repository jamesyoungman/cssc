/*
 * getopt.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 * Members of the class getopt.
 *
 */

#ifdef __GNUC__
#pragma implementation "getopt.h"
#endif

#include "cssc.h"
#include "getopt.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: getopt.cc,v 1.6 1997/07/02 18:18:09 james Exp $";
#endif

int
getopt::next()
{
  if (cindex == NULL || *cindex == '\0')
    {
      if (index >= argc)
	{
	  return END_OF_ARGUMENTS;
	}
      cindex = argv[index];

      // A dash on its own is not a valid option so we have
      // reached the end of the arguments.
      if (cindex[0] != '-' || cindex[1] == '\0')
	{
	  return END_OF_ARGUMENTS;
	}
      index++;

      // The ragument "--" is the POSIX signal for end-of-args.
      if (cindex[1] == '-' && cindex[2] == '\0')
	{
	  return END_OF_ARGUMENTS;
	}
      cindex++;
    }
  // Collect the argument character.
  char c = *cindex++;

  // Look for the argument character in the option list.
  char *match = strchr(opts, c);
	
  if (c == '\0' || match == NULL)
    {
      if (opterr)		// set opterr for verbose error returns.
	{
	  quit(-2, "Unrecognized option '%c'.\n", c);
	}
      arg = cindex - 1;
      return UNRECOGNIZED_OPTION;
    }

  // match[1] may contain either a modifier for an option,
  // or the next option letter in the option list.

  // If an option letter is followed by "!", this indicates
  // that any argument must follow the option letter immediately.
  int takes_arg;
  int arg_catenated;

  if (match[1] == ':')		// option takes an argument.
    {
      takes_arg = 1;
      arg_catenated = 0;
    }
  else if (match[1] == '!')	// option takes a concatenated argument.
    {
      takes_arg = 1;
      arg_catenated = 1;
    }
  else				// no argument.
    {
      takes_arg = arg_catenated = 0;
    }
  
  
  if (takes_arg)
    {
      if (*cindex == '\0' && (!arg_catenated) )
	{
	  // Option is of the form "-X", the argument is next
	  // in the list. 
	  if (index >= argc)
	    {
	      if (opterr)
		{
		  quit(-2, "Option '%c' requires"
		       "an argument.\n", c);
		}
	      arg = cindex - 1;
	      return MISSING_ARGUMENT;
	    }
	  arg = argv[index++];
	}
      else
	{
	  // Option is of the form "-Xmumble", the argument
	  // is "mumble".  Here, *cindex could be '\0', meaning
	  // that the option is a "catenated arg" option like 
	  // sccs-get's -y option, and there is no actual argument.
	  arg = cindex;
	}
      cindex = NULL;
    }
  else				
    {
      arg = NULL;		// no argument taken by this option.
    }

  return c;
}

int getopt::get_index(void) const
{
  return index;
}

char *getopt::getarg(void) const
{
  return arg;
}

/* Local variables: */
/* mode: c++ */
/* End: */
