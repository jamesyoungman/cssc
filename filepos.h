/* filepos.h: Part of GNU CSSC. -*- C++ -*- Class FilePosSaver
 * 
 * 
 *    Copyright (C) 1997,1998 Free Software Foundation, Inc. 
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
 */

// We use fgetpos()/fsetpos() if available, else ftell()/fseek().
// On systems where the two methods are distinct, both will surely
// be provided?



#ifndef CSSC__FILEPOS_H
#define CSSC__FILEPOS_H "$Id: filepos.h,v 1.7 1998/09/02 21:03:25 james Exp $"

#include "cssc.h"

// SunOS requires <unistd.h> for SEEK_SET, for some bizarre reason.
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif



class FilePosSaver		// with fsetpos()...
{
  FILE *f;
  int disarmed;

#ifdef HAVE_FSETPOS
  fpos_t pos;
  
 public:
  FilePosSaver(FILE *fp) : f(fp), disarmed(0)
    {
      if (0 != fgetpos(f, &pos))
	ctor_fail(errno, "fgetpos() failed!");
      // better, later; throw exception.
    }

  ~FilePosSaver()		// and restore it in the destructor.
    {
      if (!disarmed)
	if (0 != fsetpos(f, &pos))
	  ctor_fail(errno, "fsetpos() failed!");
    }

#else
  long   offset;

 public:
  FilePosSaver(FILE *fp) : f(fp), disarmed(0)
    {
      if (-1L == (offset = ftell(f)) )
	ctor_fail(errno, "ftell() failed."); // better, later; throw exception.
    }

  ~FilePosSaver()		// and restore it in the destructor.
    {
      if (!disarmed)
	if (0 != fseek(f, offset, SEEK_SET))
	  ctor_fail(errno, "fseek() failed!");
    }

#endif
  
 public:  
  
  void disarm()			// turn off the restore at the destructor.
  {
    disarmed = 1;
  }
};


#endif
