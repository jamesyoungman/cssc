/*
 * relvbr.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019, 2024
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
 * Members of the class sccs_file for doing sccs-prt.
 *
 */


#ifndef INC_RELVBR_H
#define INC_RELVBR_H

#include <cstdio>

class sid;

class relvbr
{
  short rel_;
  short level_;
  short branch_;

public:
  relvbr()
  : rel_(-1),
    level_(-1),
    branch_(-1)
  {
  }
  relvbr(short int r, short int l, short int b)
  : rel_(r),
    level_(l),
    branch_(b)
  {
  }
  relvbr(const char *s);
  relvbr(const sid& s);

  int valid() const { return ((rel_ > 0) && (level_ > 0) && (branch_ > 0)); }

  enum { LARGEST = 9999 };	// largest valid relvbr number.

  bool operator<(const relvbr& r2) const
    {
	return ((rel_ < r2.rel_) ||
		((rel_ == r2.rel_) &&
		 ((level_ < r2.level_) ||
		  ((level_ == r2.level_) && (branch_ < r2.branch_)))));
    }

  bool operator>(const relvbr& r2) const
    {
	return ((rel_ > r2.rel_) ||
		((rel_ == r2.rel_) &&
		 ((level_ > r2.level_) ||
		  ((level_ == r2.level_) && (branch_ > r2.branch_)))));
    }

  bool operator<=(const relvbr& r2) const
    {
      return ((rel_ < r2.rel_) ||
		((rel_ == r2.rel_) &&
		 ((level_ < r2.level_) ||
		  ((level_ == r2.level_) && (branch_ <= r2.branch_)))));
    }

  bool operator>=(const relvbr& r2) const
    {
      return ((rel_ > r2.rel_) ||
		((rel_ == r2.rel_) &&
		 ((level_ > r2.level_) ||
		  ((level_ == r2.level_) && (branch_ >= r2.branch_)))));
    }

  bool operator==(const relvbr& r2) const
    {
      return ((rel_    == r2.rel_   ) &&
	      (level_  == r2.level_ ) &&
	      (branch_ == r2.branch_));
    }

  bool operator!=(const relvbr& r2) const
    {
      return ((rel_    != r2.rel_   ) ||
	      (level_  != r2.level_ ) ||
	      (branch_ != r2.branch_));
    }

  int print(FILE *out) const { return (fprintf(out, "%d.%d.%d", rel_, level_, branch_) < 0); }
};


/* Local variables: */
/* mode: c++ */
/* End: */
#endif
