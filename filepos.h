// -*- C++ -*- Class for saving and restoring our file position.

// We use fgetpos()/fsetpos() if available, else ftell()/fseek().
// On systems where the two methods are distinct, both will surely
// be provided?



#include "cssc.h"

#ifndef CSSC__FILEPOS_H
#define CSSC__FILEPOS_H

// SunOS requires <unistd.h> for SEEK_SET, for some bizarre reason.
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#ifdef HAVE_FSETPOS

class FilePosSaver		// with fsetpos()...
{
public:  
  FilePosSaver(FILE*);		// save the position on this FILE...
  ~FilePosSaver();		// and restore it in the destructor.
private:

  fpos_t pos;
  FILE *f;
};

FilePosSaver::FilePosSaver(FILE *fp) : f(fp)
{
  if (0 != fgetpos(f, &pos))
    quit(errno, "fgetpos() failed!"); // better, later; throw exception.
}

FilePosSaver::~FilePosSaver()
{
  if (0 != fsetpos(f, &pos))
    quit(errno, "fsetpos() failed!");
}

#else

class FilePosSaver		// no fsetpos()...
{
public:  
  FilePosSaver(FILE*);		// save the position on this FILE...
  ~FilePosSaver();		// and restore it in the destructor.
private:

  long   offset;
  FILE   *f;
};


FilePosSaver::FilePosSaver(FILE *fp) : f(fp)
{
  offset = ftell(f);
  if (offset == -1L)
    quit(errno, "ftell() failed.");
}

FilePosSaver::~FilePosSaver()
{
  if (fseek(f, offset, SEEK_SET) != 0)
    quit(errno, "fseek() failed!");
}


#endif

#endif
