/*
 * fnsplit.cc: Part of GNU CSSC.
 * 
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
 * Functions for canonifying filenames.
 */

#include "cssc.h"
#include "mystring.h"

void
split_filename(const mystring &fullname,
	       mystring& dirname, mystring& basename)
{
  ASSERT(fullname.length() > 0);
  
  dirname = mystring("");	// empty string.
  basename = fullname;
  
  /* Find the final slash.
   */
  mystring::size_type i = fullname.find_last_of('/');
  if (i != mystring::npos)
    {
      dirname = fullname.substr(0, i+1); // initial i characters
      basename = fullname.substr(i+1, mystring::npos);
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
      mystring d, b;
      split_filename(argv[i], d, b);
      printf("Directory part=\"%s\"\nBase part=\"%s\"\n",
	     d.c_str(), b.c_str());
    }
  return 0;
}

#endif
