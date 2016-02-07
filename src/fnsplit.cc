/*
 * fnsplit.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc.
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
#include "cssc.h"
#include "file.h"		// declaration of split_filename.
#include "cssc-assert.h"

void
split_filename(const std::string &fullname,
	       std::string& dirname, std::string& basename)
{
  ASSERT(fullname.length() > 0);

  dirname = std::string("");	// empty string.
  basename = fullname;

  /* Find the final slash.
   */
  std::string::size_type i = fullname.find_last_of('/');
  if (i != std::string::npos)
    {
      dirname = fullname.substr(0, i+1); // initial i characters
      basename = fullname.substr(i+1, std::string::npos);
      return;
    }
}

#ifdef TEST_FNSPLIT

void usage()
{
}

int main(int argc, char *argv[])
{
  for (int i=0; i<argc; ++i)
    {
      printf("Splitting \"%s\"..\n", argv[i]);
      std::string d, b;
      split_filename(argv[i], d, b);
      printf("Directory part=\"%s\"\nBase part=\"%s\"\n",
	     d.c_str(), b.c_str());
    }
  return 0;
}

#endif
