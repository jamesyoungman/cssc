/*
 * sccsdate.c: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the class sccs_date.
 */

#ifndef CSSC__SCCSDATE_H__
#define CSSC__SCCSDATE_H__

#include <string>
#include <cstdio>

#include "failure.h"
#include "quit.h"

class sccs_date
{
  int year;
  int month;
  int month_day;

  int hour;
  int minute;
  int second;

  // derived data
  int yearday;			// days since start of year.

public:
  sccs_date();
  sccs_date(const char *cutoff);
  sccs_date(const char *date, const char *time);
  sccs_date(int yr, int mth, int day,
	    int hr, int min, int sec);

  bool valid() const;

  static sccs_date now();
  std::string as_string() const;

  cssc::Failure printf(FILE *f, char fmt) const;
  cssc::Failure print(FILE *f) const;

  bool operator >(sccs_date const &) const;
  bool operator <(sccs_date const &) const;
  bool operator <=(sccs_date const &) const;

private:
  inline int compare(sccs_date const &) const;
  void update_yearday();
};


#endif /* __SCCSDATE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
