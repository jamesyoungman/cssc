/*
 * my-getopt.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998,1999, Free Software Foundation, Inc. 
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
 * Members of the class CSSC_Options.
 *
 */

/* SCCS insists that options taking arguments have their arguments
 * immediately following the keyletter.  Most Unix programs allow the
 * argument to appear as the following element in argv[], but SCCS is
 * different.  We aim for total compatibility with SCCS (except the
 * error messages and any size limitations wwe might find).  So we
 * expect the same.  This means that the usual character used in
 * getopt() to indicate that the keyletter takes an argument (":") is
 * not supported.  We use "!" instead, to indicate that this is not
 * the traditional Unix behaviour.  You can define
 * INCOMPATIBLE_OPTIONS if you wish to support a new option that has
 * the traditional Unix semantics rather than the normal SCCS
 * semantics, but I [jay@gnu.org]do not reccomend this.
 */
#undef INCOMPATIBLE_OPTIONS


/* We do, however, support the POSIX idiom of "--" signifying the end
 * of the options and the beginning of the filename list.  I don't
 * think that this presents a real incompatibility problem.
 */

#ifdef __GNUC__
#pragma implementation "my-getopt.h"
#endif

//#include "cssc.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>      /* for exit() */
#endif

#ifdef HAVE_STRING_H
#include <string.h>      /* for strchr() */
#endif

#include "my-getopt.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: my-getopt.cc,v 1.8 2001/09/29 19:39:41 james_youngman Exp $";
#endif

int
CSSC_Options::next()
{
  if (cindex == 0 || *cindex == '\0')
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

      // The argument "--" is the POSIX signal for end-of-args.
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
	
  if (0 == c || 0 == match)
    {
      if (opterr)		// set opterr for verbose error returns.
	{
	  fprintf(stderr, "Unrecognized option '%c'.\n", c);
	  exit(2);
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

  takes_arg = arg_catenated = 0;

#if INCOMPATIBLE_OPTIONS
  if (match[1] == ':')		// option takes an argument.
    {
      takes_arg = 1;
      arg_catenated = 0;
    }
#else
  // Check for and warn about colons in the argument spec!
  if (NULL != strchr(opts, ':'))
    {
      fprintf(stderr,
	      "Programmer error: option list contains a colon.\n"
	      "This is illegal for SCCS. Please see "
	      __FILE__
	      ", line number %d in the source code\n", 
	      __LINE__);
      exit(3);
    }
#endif

  if (match[1] == '!')	// option takes a concatenated argument.
    {
      takes_arg = 1;
      arg_catenated = 1;
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
		  fprintf(stderr, "Option '%c' requires an argument.\n", c);
		  exit(2);
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
      cindex = 0;
    }
  else				
    {
      arg = 0;			// no argument taken by this option.
    }

  return c;
}

int CSSC_Options::get_index(void) const
{
  return index;
}

char *CSSC_Options::getarg(void) const
{
  return arg;
}

/*
 * reorder: rearrange the argv[] array so that the options
 *          come before the filenames.  The internal ordering
 *          of the options and of the filenames is unchanged.
 */
void CSSC_Options::reorder(void)
{
  int i;
  char **files, **options;
  files   = new char*[argc];
  options = new char*[argc];
  
  // separate the options from the files.
  // we lease argv[0] alone...
  for (i=1; i<argc; ++i)
    {
      if ('-' == argv[i][0])
	{
	  if ('-' == argv[i][1])
	    {
	      // -- signals end of options; stuff the remaining options
	      // into the files list.
	      files[i] = 0;
	      options[i] = 0;
	      for(++i; i<argc; ++i)
		{
		  files[i] = argv[i];
		  options[i] = 0;
		}
	    }
	  else
	    {
	      // normal option.
	      options[i] = argv[i];
	      files[i] = 0;
	    }
	}
      else
	{
	  files[i] = argv[i];
	  options[i] = 0;
	}
    }

  // now merge them, options first then files.
  // we leave argv[0] alone.
  int n = 1;
  for (i=1; i<argc; ++i)
    {
      if (options[i])
	argv[n++] = options[i];
    }
  for (i=1; i<argc; ++i)
    {
      if (files[i])
	argv[n++] = files[i];
    }
  for (i=n; i<argc; ++i)
    {
      argv[i] = 0;
    }
  
  argc = n;			// we might have removed some...

#if 0  
  fprintf(stderr, "** args [%d]: ", argc);
  for (i=0; i<argc; i++)
    {
      fprintf(stderr, "%s ", argv[i]);
    }
  fprintf(stderr, "\n");
#endif
  
  delete[] files;
  delete[] options;
  
}

CSSC_Options::CSSC_Options(int ac, char **av, const char *s, int err)
  : argc(ac),
    argv(av),
    index(1),
    cindex(0),
    opts(s),
    opterr(err)
{
  reorder();
}

int CSSC_Options::get_argc(void) const
{
  return argc;
}

char **CSSC_Options::get_argv(void) const
{
  return argv;
}


/* Local variables: */
/* mode: c++ */
/* End: */
