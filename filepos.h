// -*- C++ -*- Class for saving and restoring our file position.

// We use fgetpos()/fsetpos() if available, else ftell()/fseek().
// On systems where the two methods are distinct, both will surely
// be provided?



#ifndef CSSC__FILEPOS_H
#define CSSC__FILEPOS_H "$Id: filepos.h,v 1.4 1998/01/25 22:33:01 james Exp $"

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
	quit(errno, "fgetpos() failed!"); // better, later; throw exception.
    }

  ~FilePosSaver()		// and restore it in the destructor.
    {
      if (!disarmed)
	if (0 != fsetpos(f, &pos))
	  quit(errno, "fsetpos() failed!");
    }

#else
  long   offset;

 public:
  FilePosSaver(FILE *fp) : f(fp), disarmed(0)
    {
      if (-1L == (offset = ftell(f)) )
	quit(errno, "ftell() failed."); // better, later; throw exception.
    }

  ~FilePosSaver()		// and restore it in the destructor.
    {
      if (!disarmed)
	if (0 != fseek(f, offset, SEEK_SET))
	  quit(errno, "fseek() failed!");
    }

#endif
  
 public:  
  
  void disarm()			// turn off the restore at the destructor.
  {
    disarmed = 1;
  }
};


#endif
