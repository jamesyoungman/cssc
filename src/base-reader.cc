/*
 * base-reader.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 2019 Free Software Foundation, Inc.
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
 */
#include "config.h"

#include <ctype.h>

#include "cssc.h"
#include "base-reader.h"
#include "quit.h"

unsigned short strict_atous(const sccs_file_location& loc, const char *s)
{
  long n = 0;
  bool empty = true;
  char c;
  while ( 0 != (c=*s++) )
    {
      if (!isdigit((unsigned char)c))
	{
	  corrupt(loc, "Invalid number");
	}
      empty = false;
      n = n * 10 + (c - '0');
      if (n > 65535L)
	{
	  corrupt(loc, "Number too big");
	}
    }
  if (empty)
    {
      corrupt(loc, "Missing number");
    }
  return (unsigned short) n;
}

cssc::Failure
sccs_file_reader_base::copy_to(FILE* out)
{
  enum { BufSize = 8192 };
   std::unique_ptr<char[]> buf{new char[BufSize]};
   size_t nread;
   while ((nread=fread(buf.get(), 1, BufSize, f_)) != 0)
     {
       const size_t nwritten = fwrite(buf.get(), 1, nread, out);
       if (nwritten < nread)
	 {
	   return cssc::make_failure_builder_from_errno(errno)
	     << "short write";
	 }
     }
   if (ferror(f_))
     {
       return cssc::make_failure_builder_from_errno(errno)
	 << "read failure";
     }
   return cssc::Failure::Ok();
}
