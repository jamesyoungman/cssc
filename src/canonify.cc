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
#include <stdlib.h>		// free

#include "sccsname.h"
#include "failure.h"
#include "failure_or.h"


#include <cstring>

#include "canonicalize.h"

using std::string;

cssc::FailureOr<string>
canonify_filename(const char* fname)
{
  char *p = canonicalize_filename_mode(fname, CAN_EXISTING);
  if (nullptr == p)
    {
      return cssc::make_failure_from_errno(errno);
    }
  std::string result(p);
  free(p);
  return result;
}

/* Local variables: */
/* mode: c++ */
/* End: */
