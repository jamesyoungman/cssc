/*
 * sccsdate.c: Part of GNU CSSC.
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the class sccs_date.
 *
 * @(#) CSSC sccsdate.h 1.1 93/11/09 17:17:49
 *
 */

#ifndef CSSC__SCCSDATE_H__
#define CSSC__SCCSDATE_H__

#include <string>
using std::string;

#include <cstdio>

#include "ioerr.h"
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

  int valid() const;

  static sccs_date now();
  string as_string() const;

  int printf(FILE *f, char fmt) const;
  int print(FILE *f) const;

  int operator >(sccs_date const &) const;
  int operator <(sccs_date const &) const;

  int operator <=(sccs_date const &) const;

private:
  inline int compare(sccs_date const &) const;
  void update_yearday();
};


#endif /* __SCCSDATE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
