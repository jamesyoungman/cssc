/*
 * relvbr.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014 Free Software Foundation, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Members of the class sccs_file for doing sccs-prt.
 *
 */


#ifndef INC_RELVBR_H
#define INC_RELVBR_H

#include <cstdio>

class sid;

class relvbr
{
  short rel;
  short level;
  short branch;

  //  relvbr(short r, short l, short b, sid const *): rel(r,l,b) {}

public:
  relvbr()
  : rel(-1), level(-1), branch(-1)
  {
  }
  relvbr(short int r, short int l, short int b)
  : rel(r), level(l), branch(b)
  {
  }
  relvbr(const char *s);
  relvbr(const sid& s);

  int valid() const { return ((rel > 0) && (level > 0) && (branch > 0)); }

  enum { LARGEST = 9999 };	// largest valid relvbr number.

  friend int operator <(relvbr r1, relvbr r2)
    {
	return ((r1.rel < r2.rel) ||
		((r1.rel == r2.rel) &&
		 ((r1.level < r2.level) ||
		  ((r1.level == r2.level) && (r1.branch < r2.branch)))));
    }

  friend int operator >(relvbr r1, relvbr r2)
    {
	return ((r1.rel > r2.rel) ||
		((r1.rel == r2.rel) &&
		 ((r1.level > r2.level) ||
		  ((r1.level == r2.level) && (r1.branch > r2.branch)))));
    }

  friend int operator <=(relvbr r1, relvbr r2)
    {
      return ((r1.rel < r2.rel) ||
		((r1.rel == r2.rel) &&
		 ((r1.level < r2.level) ||
		  ((r1.level == r2.level) && (r1.branch <= r2.branch)))));
    }

  friend int operator >=(relvbr r1, relvbr r2)
    {
      return ((r1.rel > r2.rel) ||
		((r1.rel == r2.rel) &&
		 ((r1.level > r2.level) ||
		  ((r1.level == r2.level) && (r1.branch >= r2.branch)))));
    }

  friend int operator ==(relvbr r1, relvbr r2)
    {
      return ((r1.rel    == r2.rel   ) &&
	      (r1.level  == r2.level ) &&
	      (r1.branch == r2.branch));
    }

  friend int operator !=(relvbr r1, relvbr r2)
    {
      return ((r1.rel    != r2.rel   ) ||
	      (r1.level  != r2.level ) ||
	      (r1.branch != r2.branch));
    }

  int print(FILE *out) const { return (fprintf(out, "%d.%d.%d", rel, level, branch) < 0); }
};


/* Local variables: */
/* mode: c++ */
/* End: */
#endif
