/* 
 * sf-prt.cc: Part of GNU CSSC.
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
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
  release(int r): rel(r) {}
  release(const char *s);
  release(const sid& s);
  
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


