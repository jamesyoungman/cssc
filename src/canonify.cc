/*
 * canonify.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 * Functions for canonifying filenames.
 */
#include <config.h>

#include <cerrno>
#include <limits>
#include <string>
#include <vector>

#include "sccsname.h"
#include "failure.h"
#include "file.h"


#include <cstring>

#include <unistd.h>		// chdir()
#include <string.h>		// strerror
#include <stddef.h>

using std::string;

namespace
{
static
cssc::FailureOr<string::size_type>
new_cap(string::size_type current)
{
  string::size_type increment = current/2;
  if (0 == increment)
    increment = 1;
  if (std::numeric_limits<string::size_type>::max() - increment < current)
    return cssc::make_failure_from_errno(ERANGE);

  return current + increment;
}
}  // unnamed namespace

static cssc::FailureOr<string>
get_current_directory()
{
  std::vector<char> result;
  result.reserve(16);
  for (;;)
    {
      result.resize(result.capacity());
      const char *p = getcwd(result.data(), result.capacity());
      if (nullptr != p)
	{
	  return string(p);	// success!
	}

      if (errno != ERANGE)
	return cssc::make_failure_from_errno(errno);

      // The buffer was too small, increase it.
      auto updated = new_cap(result.capacity());
      if (!updated.ok())
	return updated.fail();	// Failed.
      // Ensure we make progress at every iteration.
      ASSERT(*updated > result.capacity());
      result.reserve(*updated);
    }
}


cssc::FailureOr<string>
canonify_filename(const char* fname)
{
  string dirname, basename;
  split_filename(fname, dirname, basename);
  bool changed_dir = false;

  cssc::FailureOr<string> old = get_current_directory();
  if (!old.ok())
    return old;			// failure
  const string old_dir(*old);

  if (dirname != "")
    {
      if (0 != chdir(dirname.c_str()))
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    << "unable to chdir to " << dirname;
	}
      changed_dir = true;
    }

  // Now that we have changed working directory, all return paths need
  // to restore the working directory.
  ResourceCleanup recover_cwd([old_dir, changed_dir](){
      if (!changed_dir)
	return;
      if (0 != chdir(old_dir.c_str()))
	{
	  fatal_quit(2, "unable to restore current directory to %s: %s",
		     old_dir.c_str(), strerror(errno));
	}
    });

  cssc::FailureOr<string> curr = get_current_directory();
  if (!curr.ok())
    return curr;		// failure

  string canonical_dir(*curr);
  return string(canonical_dir + string("/") + basename);
}




/* Local variables: */
/* mode: c++ */
/* End: */
