/*
 * my-getopt.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2007, 2008, 2009, 2010, 2011, 2014,
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
 * error messages and any size limitations we might find).  So we
 * expect the same.  This means that the usual character used in
 * getopt() to indicate that the keyletter takes an argument (":") is
 * not supported.  We use "!" instead, to indicate that this is not
 * the traditional Unix behaviour.  You can define
 * INCOMPATIBLE_OPTIONS if you wish to support a new option that has
 * the traditional Unix semantics rather than the normal SCCS
 * semantics, but I [jay@gnu.org] do not reccomend this.
 */
#undef INCOMPATIBLE_OPTIONS


/* We do, however, support the POSIX idiom of "--" signifying the end
 * of the options and the beginning of the filename list.  I don't
 * think that this presents a real incompatibility problem.
 */

#include <config.h>

#include <cstdio>
#include <stdlib.h>      /* for exit() */
#include <string.h>      /* for strchr() */


#include "my-getopt.h"
#include "quit.h"

int
CSSC_Options::next()
{
  if (cindex_ == nullptr || *cindex_ == '\0')
    {
      if (index_ >= argc_)
	{
	  return END_OF_ARGUMENTS;
	}
      cindex_ = argv_[index_];

      // A dash on its own is not a valid option so we have
      // reached the end of the arguments.
      if (cindex_[0] != '-' || cindex_[1] == '\0')
	{
	  return END_OF_ARGUMENTS;
	}
      index_++;

      // The argument "--" is the POSIX signal for end-of-args.
      if (cindex_[1] == '-' && cindex_[2] == '\0')
	{
	  return END_OF_ARGUMENTS;
	}
      cindex_++;
    }
  // Collect the argument character.
  char c = *cindex_++;

  // Look for the argument character in the option list.
  const char *match = strchr(opts_, c);

  if (0 == c || nullptr == match)
    {
      if (opterr_)		// set opterr for verbose error returns.
	{
	  fprintf(stderr, "Unrecognized option '%c'.\n", c);
	  usage();
	  exit(opterr_);
	}
      arg_ = cindex_ - 1;
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
  if (nullptr != strchr(opts_, ':'))
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
      if (*cindex_ == '\0' && (!arg_catenated) )
	{
	  // Option is of the form "-X", the argument is next
	  // in the list.
	  if (index_ >= argc_)
	    {
	      if (opterr_)
		{
		  fprintf(stderr, "Option '%c' requires an argument.\n", c);
		  usage();
		  exit(opterr_);
		}
	      arg_ = cindex_ - 1;
	      return MISSING_ARGUMENT;
	    }
	  arg_ = argv_[index_++];
	}
      else
	{
	  // Option is of the form "-Xmumble", the argument
	  // is "mumble".  Here, *cindex_ could be '\0', meaning
	  // that the option is a "catenated arg" option like
	  // sccs-get's -y option, and there is no actual argument.
	  arg_ = cindex_;
	}
      cindex_ = nullptr;
    }
  else
    {
      arg_ = nullptr;			// no argument taken by this option.
    }

  return c;
}

int CSSC_Options::get_index(void) const
{
  return index_;
}

char *CSSC_Options::getarg(void) const
{
  return arg_;
}

/*
 * reorder: rearrange the argv_[] array so that the options
 *        come before the filenames.  The internal ordering
 *        of the options and of the filenames is unchanged.
 */
void CSSC_Options::reorder(void)
{
  int i;
  char **files, **options;
  files   = new char*[argc_];
  options = new char*[argc_];

  // separate the options from the files.
  // we leave argv_[0] alone...
  for (i=1; i<argc_; ++i)
    {
      if ('-' == argv_[i][0])
	{
	  if ('-' == argv_[i][1])
	    {
	      // -- signals end of options; stuff the remaining options
	      // into the files list.
	      files[i] = nullptr;
	      options[i] = nullptr;
	      for(++i; i<argc_; ++i)
		{
		  files[i] = argv_[i];
		  options[i] = nullptr;
		}
	    }
	  else
	    {
	      // normal option.
	      options[i] = argv_[i];
	      files[i] = nullptr;
	    }
	}
      else
	{
	  files[i] = argv_[i];
	  options[i] = nullptr;
	}
    }

  // now merge them, options first then files.
  // we leave argv_[0] alone.
  int n = 1;
  for (i=1; i<argc_; ++i)
    {
      if (options[i])
	argv_[n++] = options[i];
    }
  for (i=1; i<argc_; ++i)
    {
      if (files[i])
	argv_[n++] = files[i];
    }
  for (i=n; i<argc_; ++i)
    {
      argv_[i] = nullptr;
    }

  argc_ = n;			// we might have removed some...

  delete[] files;
  delete[] options;

}

CSSC_Options::CSSC_Options(int ac, char **av, const char *s, int err)
  : argc_(ac),
    argv_(av),
    index_(1),
    cindex_(nullptr),
    opts_(s),
    opterr_(err),
    arg_(nullptr)
{
  reorder();
}

int CSSC_Options::get_argc(void) const
{
  return argc_;
}

char **CSSC_Options::get_argv(void) const
{
  return argv_;
}


/* Local variables: */
/* mode: c++ */
/* End: */
