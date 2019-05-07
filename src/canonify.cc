/*
 * canonify.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2007,2008, 2009, 2010, 2011, 2014,  Free Software Foundation, Inc.
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
 * Functions for canonifying filenames.
 */
#include <config.h>
#include <string>

#include "sccsname.h"
#include "file.h"


#include <cstring>

#include <unistd.h>		// chdir()
#include <stddef.h>

using std::string;

static string
get_current_directory()
{
  size_t len = 1;
  char *p;

  for (;;)
    {
      if (NULL != (p = new char[len]))
	{
	  const char *q;
	  if ( NULL != (q=getcwd(p, len)) )	// success!
	    {
	      string ret(q);
	      delete[] p;
	      return ret;
	    }
	  else
	    {
	      len *= 2;		// try a larger buffer.
	    }
	  delete[] p;
	}
      else			// allocation failed.
	{
	  return ".";		// this is a cop-out really.
	}
    }
}


string
canonify_filename(const char* fname)
{
  string dirname, basename;
  split_filename(fname, dirname, basename);

  string old_dir(get_current_directory());
  chdir(dirname.c_str());
  string canonical_dir(get_current_directory());
  chdir(old_dir.c_str());

  return string(canonical_dir + string("/") + basename);
}




/* Local variables: */
/* mode: c++ */
/* End: */
