/* 
 * sf-prt.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997  Free Software Foundation, Inc.
 *                     675 Mass Ave, Cambridge, MA 02139, USA
 *
 * Members of the class sccs_file for doing sccs-prt.
 *
 */


#ifndef INC_RELEASE_H
#define INC_RELEASE_H

class sid;
//#include "sid.h"

class release
{
  short rel;

  //  release(short r, sid const *): rel(r) {}

public:
  release(): rel(-1) {}
  release(short r): rel(r) {}
  release(const char *s);

  int valid() const { return rel > 0; }

  enum { LARGEST = 9999 };	// largest valid release number.
  
  release &operator++() { rel++; return *this; }
  release &operator--() { rel--; return *this; }

  operator unsigned long() const { return rel; }
  operator short()         const { return rel; }
  
  friend int operator <(release r1, release r2)
    {
      return r1.rel < r2.rel;
    }

  friend int operator >(release r1, release r2)
    {
      return r1.rel > r2.rel;
    }

  friend int operator <=(release r1, release r2)
    {
      return r1.rel <= r2.rel;
    }

  friend int operator >=(release r1, release r2)
    {
      return r1.rel >= r2.rel;
    }

  friend int operator ==(release r1, release r2)
    {
      return r1.rel == r2.rel;
    }

  friend int operator !=(release r1, release r2)
    {
      return r1.rel != r2.rel;
    }

  int print(FILE *out) const { return (fprintf(out, "%d", rel) < 0); }

  
  //  friend sid::operator release() const;
  //  friend sid::sid(release);
};







/* Local variables: */
/* mode: c++ */
/* End: */
#endif


