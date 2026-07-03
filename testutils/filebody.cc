/*
 * filebody.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 2026 Free Software Foundation, Inc.
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
 */

// Autotools requires us to include config.h first.
#include <config.h>

#include "filebody.h"

// C++ standard library header file includes
#include <string>

// POSIX header file includes
#include <unistd.h>
#include <sys/fcntl.h>

// gnulib header file includes
#include <error.h>

std::string
read_file_body (int fd, const char* filename)
{
  std::string result;
  static char buf[8192];
  if (0 != lseek (fd, SEEK_SET, 0))
    {
      error (1, errno, "lseek failed on %s", filename);
    }

  for (;;)
    {
      ssize_t nread = read (fd, buf, sizeof(buf));
      if (nread < 0)
	{
	  error (1, errno, "failed to read from %s", filename);
	}
      else if (0 == nread)
	{
	  break;
	}
      else
	{
	  std::string::size_type n = nread;
	  result.append(buf, n);
	}
    }

  return result;
}
