/*
 * canonify.cc: Part of GNU CSSC.
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
#include "sccsname.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>		// chdir()
#endif
#include <stddef.h>		// chdir()

static mystring
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
	      mystring ret(q, strlen(q));
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
	  return mystring(".");	// this is a cop-out really.
	}      
    }
}


mystring
canonify_filename(const char* fname)
{
  mystring dirname(".");
  mystring basename(fname);
  
  for (size_t i=strlen(fname)-1; i>0; --i)
    {
      if ('/' == fname[i])
	{
	  /* found the final slash.
	   */
	  dirname = mystring(fname, i); // initial i characters
	  basename = mystring( & ((const char*)fname)[i+1] );
	  break;
	}
    }
  mystring old_dir(get_current_directory());
  chdir(dirname);
  mystring canonical_dir(get_current_directory());
  chdir(old_dir);
  
  return mystring(canonical_dir + mystring("/") + basename);
}




/* Local variables: */
/* mode: c++ */
/* End: */
