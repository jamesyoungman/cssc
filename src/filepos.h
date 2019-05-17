/* filepos.h: Part of GNU CSSC. -*- C++ -*- Class FilePosSaver
 *
 *
 *  Copyright (C) 1997, 1998, 2007, 2008, 2009, 2010, 2011, 2014, 2019
 *  Free Software Foundation, Inc.
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
 */

// We use fgetpos()/fsetpos() if available, else ftell()/fseek().
// On systems where the two methods are distinct, both will surely
// be provided?



#ifndef CSSC__FILEPOS_H
#define CSSC__FILEPOS_H 1

#include "cssc.h"
#include <cerrno>

#include <unistd.h>



class FilePosSaver		// with fsetpos()...
{
  FILE *f_;
  bool disarmed_;

#ifdef HAVE_FSETPOS
  fpos_t pos_;
 public:
  FilePosSaver(FILE *fp)
    : f_(fp), disarmed_(false)
    {
      if (0 != fgetpos(f_, &pos_))
	ctor_fail(errno, "fgetpos() failed!");
      // better, later; throw exception.
    }

  ~FilePosSaver()		// and restore it in the destructor.
    {
      if (!disarmed_)
	if (0 != fsetpos(f_, &pos_))
	  ctor_fail(errno, "fsetpos() failed!");
    }

#else
  long   offset_;

 public:
  FilePosSaver(FILE *fp)
    : f_(fp), disarmed_(false)
    {
      if (-1L == (offset_ = ftell(f_)) )
	ctor_fail(errno, "ftell() failed."); // better, later; throw exception.
    }

  ~FilePosSaver()		// and restore it in the destructor.
    {
      if (!disarmed_)
	if (0 != fseek(f_, offset_, SEEK_SET))
	  ctor_fail(errno, "fseek() failed!");
    }

#endif

 public:

  void disarm()			// turn off the restore at the destructor.
  {
    disarmed_ = true;
  }
};

#endif
